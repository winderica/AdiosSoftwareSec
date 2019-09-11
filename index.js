const electron = require("electron");
const app = electron.app;
const BrowserWindow = electron.BrowserWindow;
let mainWindow;

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

app.on('ready', async () => {
    mainWindow = new BrowserWindow({
        width: 1280,
        height: 960,
        webPreferences: {
            nodeIntegration: true,
        }
    });
    mainWindow.webContents.openDevTools();
    await mainWindow.loadURL('http://localhost:3000');
    mainWindow.on('closed', function () {
        mainWindow = null;
    });
});
