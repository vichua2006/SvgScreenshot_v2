const { app, BrowserWindow } = require('electron');
const path = require('path');
const fs = require('fs');

if (process.getuid && process.getuid() === 0) {
  app.commandLine.appendSwitch('no-sandbox');
}

function createWindow() {
  const win = new BrowserWindow({
    width: 800,
    height: 600,
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: false
    }
  });

  win.loadFile('index.html');

  win.webContents.on('did-finish-load', () => {
    const dir = path.join(__dirname, '..', 'screenshots');
    fs.mkdirSync(dir, { recursive: true });
    const files = fs.readdirSync(dir)
      .filter(f => /\.(bmp|png|jpe?g)$/i.test(f))
      .map(f => path.join('..', 'screenshots', f));
    win.webContents.send('file-list', files);
  });
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});
