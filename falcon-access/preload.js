const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('FA', {
  config: () => ipcRenderer.invoke('config'),
  machineId: () => ipcRenderer.invoke('machineId'),
  api: (body) => ipcRenderer.invoke('api', body),
  publicIp: () => ipcRenderer.invoke('publicIp'),
  writeLicense: (product, data) => ipcRenderer.invoke('writeLicense', product, data),
  removeLicense: (product) => ipcRenderer.invoke('removeLicense', product),
  hasLicense: (product) => ipcRenderer.invoke('hasLicense', product),
  getInstalled: () => ipcRenderer.invoke('getInstalled'),
  setInstalled: (productId, version) => ipcRenderer.invoke('setInstalled', productId, version),
  checkPluginInstalled: (productName) => ipcRenderer.invoke('checkPluginInstalled', productName),
  openExternal: (url) => ipcRenderer.invoke('openExternal', url)
});
