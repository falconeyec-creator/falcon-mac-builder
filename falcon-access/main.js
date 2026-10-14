/**
 * FALCON ACCESS — main process
 * Login with your Falcon Eye Music store account, activate / update / manage
 * every Falcon Eye product on this computer (like Native Access).
 */
const { app, BrowserWindow, ipcMain, shell } = require('electron');
const path = require('path');
const fs = require('fs');
const os = require('os');
const crypto = require('crypto');
const { execSync } = require('child_process');

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   CONFIG — backend Web App URL + store URL එක මෙතන දාන්න
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
const API_URL  = 'https://script.google.com/macros/s/AKfycbxQXBj8iwMEoewXqusCYd9A5aXeILVw8bwIX_tZQg1SvxszZaiwD6ZjSFFCYFk_5nBEEg/exec';
const SITE_URL = 'https://falconeyesl.com/Falconeyeweb';

/* ------------------------------------------------------------------
 * Shared FalconEye data dir (same one the VST plugins use)
 * ------------------------------------------------------------------ */
function falconDir() {
  const d = path.join(process.env.ProgramData || 'C:\\ProgramData', 'FalconEye');
  try { fs.mkdirSync(d, { recursive: true }); } catch (e) {}
  return d;
}
function userFalconDir() {
  const d = path.join(app.getPath('appData'), 'FalconEye');
  try { fs.mkdirSync(d, { recursive: true }); } catch (e) {}
  return d;
}

/* ------------------------------------------------------------------
 * Machine ID — shared with the plugins via machine.id
 * (plugins read this file first, so app + plugin always agree)
 * ------------------------------------------------------------------ */
function computeMachineId() {
  let seed = '';
  try {
    seed = execSync(
      'reg query "HKLM\\SOFTWARE\\Microsoft\\Cryptography" /v MachineGuid',
      { encoding: 'utf8', windowsHide: true }).match(/MachineGuid\s+REG_SZ\s+(\S+)/)[1];
  } catch (e) {
    seed = os.hostname() + '|' + os.cpus().map(c => c.model).join(',') + '|' + os.totalmem();
  }
  return crypto.createHash('sha256').update('FalconEye|' + seed).digest('hex');
}

function getMachineId() {
  const locations = [path.join(falconDir(), 'machine.id'), path.join(userFalconDir(), 'machine.id')];
  for (const f of locations) {
    try {
      const v = fs.readFileSync(f, 'utf8').trim();
      if (/^[0-9a-f]{32,64}$/i.test(v)) return v;
    } catch (e) {}
  }
  const id = computeMachineId();
  for (const f of locations) { try { fs.writeFileSync(f, id); } catch (e) {} }
  return id;
}

/* ------------------------------------------------------------------
 * JUCE-compatible license files (exact same format the plugins read):
 * JSON -> XOR pad "FalconEye.Licensing.v1" -> JUCE MemoryBlock base64
 * ------------------------------------------------------------------ */
const XOR_PAD = 'FalconEye.Licensing.v1';
const JB64 = '.ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+';

function getBitRange(buf, start, numBits) {
  let res = 0, byte = start >> 3, off = start & 7, sofar = 0;
  while (numBits > 0 && byte < buf.length) {
    const n = Math.min(numBits, 8 - off);
    const mask = (0xff >> (8 - n)) << off;
    res |= ((buf[byte] & mask) >> off) << sofar;
    sofar += n; numBits -= n; byte++; off = 0;
  }
  return res;
}
function setBitRange(buf, start, numBits, value) {
  let byte = start >> 3, off = start & 7, sofar = 0;
  while (numBits > 0 && byte < buf.length) {
    const n = Math.min(numBits, 8 - off);
    const mask = ((1 << n) - 1) << off;
    buf[byte] = (buf[byte] & ~mask) | (((value >> sofar) << off) & mask);
    sofar += n; numBits -= n; byte++; off = 0;
  }
}
function juceB64Encode(buf) {
  const numChars = Math.floor((buf.length * 8 + 5) / 6);
  let out = String(buf.length) + '.';
  for (let i = 0; i < numChars; i++) out += JB64[getBitRange(buf, i * 6, 6)];
  return out;
}
function xorPad(buf) {
  const out = Buffer.from(buf);
  for (let i = 0; i < out.length; i++) out[i] ^= XOR_PAD.charCodeAt(i % XOR_PAD.length);
  return out;
}
function scramble(text) { return juceB64Encode(xorPad(Buffer.from(text, 'utf8'))); }

function licensePaths(product) {
  return [path.join(falconDir(), product + '.lic'), path.join(userFalconDir(), product + '.lic')];
}
function writeLicense(product, data) {
  const text = scramble(JSON.stringify(data));
  let ok = false;
  for (const f of licensePaths(product)) {
    try { fs.writeFileSync(f, text); ok = true; break; } catch (e) {}
  }
  return ok;
}
function removeLicense(product) {
  for (const f of licensePaths(product)) { try { fs.unlinkSync(f); } catch (e) {} }
  return true;
}
function hasLicenseFile(product) {
  return licensePaths(product).some(f => { try { return fs.existsSync(f); } catch (e) { return false; } });
}

/* ------------------------------------------------------------------
 * Installed-version registry (installed.json)
 * ------------------------------------------------------------------ */
function installedFile() { return path.join(userFalconDir(), 'installed.json'); }
function getInstalled() {
  try { return JSON.parse(fs.readFileSync(installedFile(), 'utf8')); } catch (e) { return {}; }
}
function setInstalled(productId, version) {
  const m = getInstalled();
  if (version) m[productId] = { version, at: new Date().toISOString() };
  else delete m[productId];
  fs.writeFileSync(installedFile(), JSON.stringify(m, null, 2));
  return m;
}

/* ------------------------------------------------------------------
 * Real on-disk install check (NOT self-reported) — matches the exact VST3
 * path our own installers actually use: Windows Inno Setup scripts install
 * to {commoncf64}\VST3\<Product Name>.vst3, macOS pkgs to
 * /Library/Audio/Plug-Ins/VST3/<Product Name>.vst3. Used to gate
 * Activate/show "reinstall" instead of trusting a user's confirm-dialog
 * click, which could be wrong (clicked OK without actually installing, or
 * uninstalled later).
 * ------------------------------------------------------------------ */
function isPluginInstalled(productName) {
  if (!productName) return false;
  const vst3Path = process.platform === 'darwin'
    ? path.join('/Library/Audio/Plug-Ins/VST3', productName + '.vst3')
    : path.join(process.env.CommonProgramFiles || 'C:\\Program Files\\Common Files', 'VST3', productName + '.vst3');
  try { return fs.existsSync(vst3Path); } catch (e) { return false; }
}

/* ------------------------------------------------------------------
 * IPC
 * ------------------------------------------------------------------ */
ipcMain.handle('config', () => ({ apiUrl: API_URL, siteUrl: SITE_URL, version: app.getVersion() }));
ipcMain.handle('machineId', () => getMachineId());
ipcMain.handle('checkPluginInstalled', (e, productName) => isPluginInstalled(productName));

ipcMain.handle('api', async (e, body) => {
  try {
    const res = await fetch(API_URL, {
      method: 'POST',
      headers: { 'Content-Type': 'text/plain;charset=utf-8' },
      body: JSON.stringify(body),
      redirect: 'follow'
    });
    return await res.json();
  } catch (err) { return { ok: false, error: 'network', detail: String(err) }; }
});

ipcMain.handle('publicIp', async () => {
  try {
    const r = await fetch('https://api.ipify.org?format=json');
    return (await r.json()).ip || '';
  } catch (e) { return ''; }
});

ipcMain.handle('writeLicense', (e, product, data) => writeLicense(product, data));
ipcMain.handle('removeLicense', (e, product) => removeLicense(product));
ipcMain.handle('hasLicense', (e, product) => hasLicenseFile(product));
ipcMain.handle('getInstalled', () => getInstalled());
ipcMain.handle('setInstalled', (e, productId, version) => setInstalled(productId, version));
ipcMain.handle('openExternal', (e, url) => {
  if (/^https?:\/\//i.test(url)) shell.openExternal(url);
});

/* ------------------------------------------------------------------ */
let win;
function createWindow() {
  const iconPath = path.join(__dirname, 'build', process.platform === 'win32' ? 'icon.ico' : 'icon.png');
  win = new BrowserWindow({
    width: 1120, height: 760, minWidth: 900, minHeight: 620,
    backgroundColor: '#07090e',
    title: 'Falcon Access',
    icon: fs.existsSync(iconPath) ? iconPath : undefined,
    autoHideMenuBar: true,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false
    }
  });
  win.loadFile(path.join(__dirname, 'renderer', 'index.html'));
}

const gotLock = app.requestSingleInstanceLock();
if (!gotLock) app.quit();
else {
  app.on('second-instance', () => { if (win) { if (win.isMinimized()) win.restore(); win.focus(); } });
  app.whenReady().then(createWindow);
  app.on('window-all-closed', () => app.quit());
}
