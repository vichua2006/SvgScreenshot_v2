const { ipcRenderer, clipboard, nativeImage } = require('electron');
const path = require('path');

function ImageCard({ file }) {
  const copyToClipboard = () => {
    const absPath = path.resolve(__dirname, file);
    const img = nativeImage.createFromPath(absPath);
    clipboard.writeImage(img);
  };

  return (
    <div style={{border: '1px solid #ccc', width: '200px', height: '200px', margin: '10px', padding: '10px', boxSizing: 'border-box', display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'space-between'}}>
      <img src={file} style={{maxWidth: '100%', maxHeight: '150px', objectFit: 'contain'}} />
      <button onClick={copyToClipboard}>Copy</button>
    </div>
  );
}

function App() {
  const [files, setFiles] = React.useState([]);

  React.useEffect(() => {
    ipcRenderer.on('file-list', (_event, list) => {
      setFiles(list);
    });
  }, []);

  return (
    <div style={{display: 'flex', flexWrap: 'wrap'}}>
      {files.map(f => <ImageCard key={f} file={f} />)}
    </div>
  );
}

ReactDOM.render(<App />, document.getElementById('root'));
