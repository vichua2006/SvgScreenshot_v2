const { ipcRenderer } = require('electron');

window.addEventListener('DOMContentLoaded', () => {
  ipcRenderer.on('file-list', (_event, files) => {
    const container = document.getElementById('images');
    files.forEach(file => {
      const img = document.createElement('img');
      img.src = file;
      img.style.maxWidth = '100%';
      img.style.display = 'block';
      img.style.marginBottom = '10px';
      container.appendChild(img);
    });
  });
});
