#pragma once

/*
    Falcon Eye Corporation — plugin-side license client.

    Flow:
      1. Plugin loads -> LicenseManager::isActivated() checks the local
         (machine-bound, obfuscated) license file.
      2. Not activated -> editor shows ActivationOverlay; user enters a key,
         we POST to the Apps Script server which binds it to this machine.
      3. Success -> license file saved; overlay disappears.
      4. Copying the license file to another machine fails: the stored
         machine id won't match, so the overlay comes back.

    The server URL and product id are compile-time constants supplied by
    the build (see FALCON_LICENSE_URL / FALCON_PRODUCT_ID definitions).
*/

#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>
#include <juce_gui_basics/juce_gui_basics.h>

class LicenseManager
{
public:
    LicenseManager (juce::String productId, juce::String serverUrl)
        : product (std::move (productId)), url (std::move (serverUrl)) {}

    // Shared with the Falcon Access app via machine.id so both always agree.
    // Priority: ProgramData\FalconEye\machine.id -> user AppData copy -> JUCE id
    // (whichever component runs first creates the file for the others).
    static juce::String getMachineId()
    {
        auto shared = machineIdFile();
        auto user   = userMachineIdFile();

        for (const auto& f : { shared, user })
            if (f.existsAsFile())
            {
                auto v = f.loadFileAsString().trim();
                if (v.length() >= 32)
                    return v;
            }

        auto id = juce::SHA256 (juce::SystemStats::getUniqueDeviceID().toUTF8()).toHexString();
        for (const auto& f : { shared, user })
        {
            f.getParentDirectory().createDirectory();
            f.replaceWithText (id);           // best effort; ignore failures
        }
        return id;
    }

    static juce::File machineIdFile()
    {
        return juce::File::getSpecialLocation (juce::File::commonApplicationDataDirectory)
                   .getChildFile ("FalconEye").getChildFile ("machine.id");
    }

    static juce::File userMachineIdFile()
    {
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("FalconEye").getChildFile ("machine.id");
    }

    // Machine-wide (the license is bound to the machine, not the Windows user)
    juce::File licenseFile() const
    {
        return juce::File::getSpecialLocation (juce::File::commonApplicationDataDirectory)
                   .getChildFile ("FalconEye").getChildFile (product + ".lic");
    }

    // Location used by older builds; kept as a read fallback
    juce::File legacyUserLicenseFile() const
    {
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("FalconEye").getChildFile (product + ".lic");
    }

    bool isActivated() const
    {
        auto state = readLicenseFile();
        const bool valid = state.isObject()
            && state["machineId"].toString() == getMachineId()
            && state["product"].toString() == product
            && state["token"].toString().isNotEmpty();

        // Migrate a valid per-user license to the machine-wide location
        if (valid && ! licenseFile().existsAsFile())
            writeLicenseFile (state);

        return valid;
    }

    juce::String getStoredKey() const { return readLicenseFile()["key"].toString(); }

    // NOTE: there is deliberately no plugin-side activate(enteredKey) anymore.
    // Typing a raw key directly into the plugin let anyone holding the key
    // text (e.g. bought second-hand from the real owner) activate it without
    // ever logging into the owning Falcon Eye account — bypassing Falcon
    // Access entirely, and with it the account-level abuse/ban protections
    // that live server-side. The server now rejects lic.activate calls that
    // aren't backed by a Falcon-Access session token owning the license, so
    // Falcon Access (which writes the license file to disk itself, in its
    // own matching XOR-scrambled format) is the only real activation path
    // left — this class only ever reads that file, never writes it for a
    // fresh activation.

    // Silent server-side recheck (on load + periodic via LicenseWatchdog).
    // Removes the license ONLY on definite license errors (revoked / disabled /
    // locked / moved) — network failures never punish offline users.
    // Optional callback fires on the message thread with a warning message.
    void revalidate (std::function<void (juce::String reason)> onKilled = {})
    {
        if (! isActivated())
            return;

        juce::DynamicObject::Ptr req = new juce::DynamicObject();
        req->setProperty ("action", "lic.validate");
        req->setProperty ("key", getStoredKey());
        req->setProperty ("machineId", getMachineId());
        req->setProperty ("product", product);

        auto response = postJson (juce::JSON::toString (juce::var (req.get())));
        if (! response.isObject() || (bool) response["ok"])
            return;                               // ok, or offline -> keep working

        const auto err = response["error"].toString();
        const bool fatal = err == "revoked" || err == "disabled" || err == "locked"
                        || err == "invalid_key" || err == "not_valid_here"
                        || err == "wrong_product" || err == "account_blocked";
        if (! fatal)
            return;                               // e.g. server_error -> ignore

        licenseFile().deleteFile();
        legacyUserLicenseFile().deleteFile();

        if (onKilled)
        {
            const auto msg = errorMessage (err);
            juce::MessageManager::callAsync ([onKilled, msg] { onKilled (msg); });
        }
    }

private:
    juce::String product, url;

    static juce::String errorMessage (const juce::String& code)
    {
        if (code == "invalid_key")                 return TRANS ("That license key was not recognised.");
        if (code == "wrong_product")               return TRANS ("This key belongs to a different product.");
        if (code == "revoked")                     return TRANS ("This license has been REVOKED. Please contact Falcon Eye Music.");
        if (code == "disabled")                    return TRANS ("This license has been disabled. Please contact Falcon Eye Music.");
        if (code == "locked")                 return TRANS ("This license is LOCKED. Contact Falcon Eye Music to unlock it.");
        if (code == "not_valid_here")         return TRANS ("This license is not activated on this computer. Open Falcon Access to activate.");
        if (code == "machine_limit_reached")  return TRANS ("This license is already registered on the maximum number of computers. Open Falcon Access to remove one, or contact Falcon Eye Music.");
        if (code == "account_blocked")        return TRANS ("Your Falcon Eye account has been suspended for suspicious activity. Contact Falcon Eye Music support.");
        return TRANS ("Activation failed: ") + code;
    }

    juce::var postJson (const juce::String& body) const
    {
        // Apps Script answers POSTs with a 302 to script.googleusercontent.com.
        // JUCE won't re-issue a POST across that hop, so follow it by hand
        // with a GET (which is what the redirect target expects).
        juce::WebInputStream stream (juce::URL (url).withPOSTData (body), true);
        stream.withExtraHeaders ("Content-Type: application/json")
              .withConnectionTimeout (10000)
              .withNumRedirectsToFollow (0);

        if (! stream.connect (nullptr))
            return {};

        const int status = stream.getStatusCode();
        if (status >= 300 && status < 400)
        {
            auto location = stream.getResponseHeaders()["Location"];
            if (location.isEmpty())
                return {};

            auto redirected = juce::URL (location).createInputStream (
                juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)
                    .withConnectionTimeoutMs (10000));
            if (redirected == nullptr)
                return {};
            return juce::JSON::parse (redirected->readEntireStreamAsString());
        }

        return juce::JSON::parse (stream.readEntireStreamAsString());
    }

    // --- Obfuscated storage (XOR + Base64) — deters casual editing only;
    //     real protection is the machine-id binding checked above.
    static juce::String xorScramble (const juce::String& in)
    {
        const char pad[] = "FalconEye.Licensing.v1";
        juce::MemoryBlock mb (in.toRawUTF8(), in.getNumBytesAsUTF8());
        auto* data = static_cast<char*> (mb.getData());
        for (size_t i = 0; i < mb.getSize(); ++i)
            data[i] ^= pad[i % (sizeof (pad) - 1)];
        return mb.toBase64Encoding();
    }

    static juce::String xorUnscramble (const juce::String& in)
    {
        juce::MemoryBlock mb;
        if (! mb.fromBase64Encoding (in))
            return {};
        auto* data = static_cast<char*> (mb.getData());
        const char pad[] = "FalconEye.Licensing.v1";
        for (size_t i = 0; i < mb.getSize(); ++i)
            data[i] ^= pad[i % (sizeof (pad) - 1)];
        return mb.toString();
    }

    juce::var readLicenseFile() const
    {
        for (const auto& f : { licenseFile(), legacyUserLicenseFile() })
            if (f.existsAsFile())
            {
                auto v = juce::JSON::parse (xorUnscramble (f.loadFileAsString()));
                if (v.isObject())
                    return v;
            }
        return {};
    }

    void writeLicenseFile (const juce::var& v) const
    {
        const auto text = xorScramble (juce::JSON::toString (v));

        auto target = licenseFile();
        target.getParentDirectory().createDirectory();
        if (target.replaceWithText (text))
            return;

        // ProgramData not writable (locked-down machines) -> per-user fallback
        auto fallback = legacyUserLicenseFile();
        fallback.getParentDirectory().createDirectory();
        fallback.replaceWithText (text);
    }
};

//==============================================================================
/**
    Full-window overlay shown until the plugin is activated.

    Deliberately has NO key-entry field. Digital licenses can only be
    activated by signing into the owning account in the FALCON ACCESS app —
    a raw key-entry box here would let anyone holding just the key text
    (e.g. bought second-hand from the real owner) activate the plugin
    without ever touching Falcon Access, bypassing the account-level
    abuse/ban protections entirely. This overlay just polls the local
    license file (cheap — no network call) so it auto-dismisses the moment
    Falcon Access finishes activating, without needing the plugin reloaded.
*/
class ActivationOverlay : public juce::Component,
                          private juce::Timer
{
public:
    ActivationOverlay (LicenseManager& lm, std::function<void()> onActivatedFn)
        : licenses (lm), onActivated (std::move (onActivatedFn))
    {
        // Tell JUCE this component always paints every one of its pixels —
        // without this, background-repaint optimisations can skip re-drawing
        // the dark scrim when something underneath invalidates itself, which
        // is what made the overlay look "see-through" over the knobs.
        setOpaque (true);
        startTimer (1500);
    }

    /** Show a server-side warning (e.g. from LicenseWatchdog when revoked). */
    void setStatusText (const juce::String& text)
    {
        statusText = text;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        // Backdrop: subtle vertical depth instead of a flat fill.
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0b0e14), 0.0f, 0.0f,
                                                  juce::Colour (0xff141a26), 0.0f, (float) getHeight(), false));
        g.fillRect (getLocalBounds());

        auto card = getCardBounds();

        juce::DropShadow (juce::Colour::fromRGBA (0, 0, 0, 130), 32, { 0, 12 }).drawForRectangle (g, card.toNearestInt());

        g.setColour (juce::Colour (0xff141a26));
        g.fillRoundedRectangle (card, 16.0f);
        g.setColour (juce::Colour (0xff262f42));
        g.drawRoundedRectangle (card, 16.0f, 1.2f);

        // Floating lock badge straddling the card's top edge.
        auto badge = juce::Rectangle<float> (54.0f, 54.0f).withCentre ({ card.getCentreX(), card.getY() });
        g.setGradientFill (juce::ColourGradient (accentColour, badge.getX(), badge.getY(),
                                                  accentColour2, badge.getRight(), badge.getBottom(), false));
        g.fillEllipse (badge);
        {
            auto lock = badge.reduced (17.0f);
            juce::Path shackle;
            shackle.addArc (lock.getX() + 2.0f, lock.getY(), lock.getWidth() - 4.0f, lock.getWidth() - 4.0f,
                             juce::MathConstants<float>::pi, juce::MathConstants<float>::twoPi, true);
            g.setColour (juce::Colour (0xff14100a));
            g.strokePath (shackle, juce::PathStrokeType (2.2f));
            g.fillRoundedRectangle (lock.getX(), lock.getCentreY() - 1.0f, lock.getWidth(), lock.getHeight() * 0.6f, 2.5f);
        }

        auto textArea = card.reduced (26.0f);
        textArea.removeFromTop (32.0f); // clear the badge overlap

        g.setColour (juce::Colour (0xffeef1f7));
        g.setFont (juce::Font (juce::FontOptions (20.0f, juce::Font::bold)));
        g.drawText ("ACTIVATE YOUR LICENSE", textArea.removeFromTop (30.0f), juce::Justification::centred);

        g.setColour (statusText.isEmpty() ? juce::Colour (0xff8b97ab) : accentColour);
        g.setFont (juce::Font (juce::FontOptions (13.0f)));
        g.drawFittedText (statusText.isEmpty()
                        ? juce::String ("Open FALCON ACCESS, sign in, and click Activate.\n"
                                         "This product can only be activated through Falcon Access.")
                        : statusText,
                    textArea.removeFromTop (76.0f).reduced (10.0f, 0.0f).toNearestInt(),
                    juce::Justification::centred, 3);
    }

    void resized() override {}

private:
    /** Centred card the whole overlay is drawn on; shared by paint() and resized()
        so the controls always line up with what's actually painted. */
    juce::Rectangle<float> getCardBounds() const
    {
        auto b = getLocalBounds().toFloat();
        const float w = juce::jmin (380.0f, b.getWidth() - 48.0f);
        const float h = juce::jmin (220.0f, b.getHeight() - 48.0f);
        return juce::Rectangle<float> (w, h).withCentre (b.getCentre());
    }

    void timerCallback() override
    {
        // Falcon Access writes the license file on its own timeline, entirely
        // outside this process — poll for it rather than needing the plugin
        // reloaded after the user activates.
        if (licenses.isActivated())
        {
            stopTimer();
            setVisible (false);
            if (onActivated)
                onActivated();
        }
    }

    const juce::Colour accentColour  { 0xfff5b301 };
    const juce::Colour accentColour2 { 0xffff7a00 };

    LicenseManager& licenses;
    std::function<void()> onActivated;
    juce::String statusText;
};

//==============================================================================
/**
    Periodic online license enforcement ("realtime" revoke).

    Owned by the plugin editor/processor. Runs revalidate() on a background
    thread every `intervalMinutes` while the plugin is loaded. The moment the
    server reports revoked / disabled / locked / moved, the local license file
    is deleted and `onKilled` fires on the message thread — show the
    ActivationOverlay again with the warning message.

    Offline users are never interrupted: network failures are ignored, and the
    next successful online check applies whatever the server says.

    Usage (in the editor):
        watchdog = std::make_unique<LicenseWatchdog> (licenseManager, [this] (juce::String why)
        {
            overlay.setStatusText (why);   // or your own warning UI
            overlay.setVisible (true);
        });
*/
class LicenseWatchdog : private juce::Timer,
                        private juce::Thread
{
public:
    LicenseWatchdog (LicenseManager& lm,
                     std::function<void (juce::String reason)> onKilledFn,
                     int intervalMinutes = 5)
        : juce::Thread ("FalconWatchdog"),
          licenses (lm), onKilled (std::move (onKilledFn))
    {
        startTimer (juce::jmax (1, intervalMinutes) * 60 * 1000);
        startThread();   // also check once shortly after load
    }

    ~LicenseWatchdog() override
    {
        stopTimer();
        stopThread (12000);
    }

private:
    void timerCallback() override
    {
        if (! isThreadRunning())
            startThread();
    }

    void run() override
    {
        licenses.revalidate (onKilled);
    }

    LicenseManager& licenses;
    std::function<void (juce::String)> onKilled;
};
