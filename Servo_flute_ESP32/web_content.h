/***********************************************************************************************
 * web_content.h - Interface web embarquee en PROGMEM
 *
 * Single Page Application avec 4 onglets :
 * 1. Clavier virtuel (14 notes jouables + velocity)
 * 2. Lecteur MIDI (upload + transport)
 * 3. Configuration (editable + sauvegarde JSON sur LittleFS)
 * 4. Monitoring temps reel (WebSocket)
 ***********************************************************************************************/
#ifndef WEB_CONTENT_H
#define WEB_CONTENT_H

const char WEB_HTML_CONTENT[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>ServoFlute Control</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;
background:#1a1a2e;color:#e0e0e0;overflow-x:hidden}
.header{background:#16213e;padding:12px 16px;display:flex;justify-content:space-between;
align-items:center;border-bottom:2px solid #0f3460}
.header h1{font-size:1.2em;color:#e94560}
.status-dot{width:10px;height:10px;border-radius:50%;display:inline-block;margin-right:6px}
.status-dot.on{background:#4ecca3}.status-dot.off{background:#e94560}
.status-bar{font-size:0.85em;color:#a0a0a0}
nav{display:flex;background:#16213e;border-bottom:1px solid #0f3460}
nav button{flex:1;padding:10px 6px;border:none;background:transparent;color:#a0a0a0;
font-size:0.85em;cursor:pointer;border-bottom:2px solid transparent;transition:all .2s}
nav button.active{color:#e94560;border-bottom-color:#e94560;background:#1a1a2e}
nav button:hover{color:#fff}
.tab{display:none;padding:16px;max-width:800px;margin:0 auto}
.tab.active{display:block}

/* Clavier */
.keyboard{display:flex;flex-wrap:wrap;gap:6px;justify-content:center;margin:16px 0}
.key{width:calc(25% - 6px);min-width:70px;padding:14px 8px;border:2px solid #0f3460;
border-radius:8px;background:#16213e;color:#e0e0e0;font-size:0.9em;text-align:center;
cursor:pointer;user-select:none;-webkit-user-select:none;transition:all .1s}
.key:active,.key.pressed{background:#e94560;border-color:#e94560;color:#fff;transform:scale(0.95)}
.key .note-name{font-weight:bold;font-size:1.1em;display:block}
.key .note-midi{font-size:0.75em;color:#888;display:block;margin-top:2px}
.key.black{background:#0a0a1a;border-color:#333}
.key.black:active,.key.black.pressed{background:#c0392b;border-color:#c0392b}
.octave-label{width:100%;text-align:center;font-size:0.8em;color:#666;margin:8px 0 4px}

/* Velocity slider */
.slider-group{display:flex;align-items:center;gap:12px;margin:16px 0;padding:12px;
background:#16213e;border-radius:8px}
.slider-group label{min-width:80px;font-size:0.9em}
.slider-group input[type=range]{flex:1;accent-color:#e94560}
.slider-group .val{min-width:40px;text-align:right;font-weight:bold;color:#e94560}

/* Player */
.upload-zone{border:2px dashed #0f3460;border-radius:12px;padding:30px;text-align:center;
margin:16px 0;cursor:pointer;transition:all .2s}
.upload-zone:hover,.upload-zone.dragover{border-color:#e94560;background:rgba(233,69,96,0.1)}
.upload-zone input{display:none}
.file-info{background:#16213e;border-radius:8px;padding:12px;margin:12px 0;display:none}
.file-info.visible{display:block}
.transport{display:flex;gap:8px;justify-content:center;margin:16px 0}
.transport button{padding:10px 24px;border:2px solid #0f3460;border-radius:8px;
background:#16213e;color:#e0e0e0;font-size:1em;cursor:pointer;transition:all .2s}
.transport button:hover{border-color:#e94560;color:#e94560}
.transport button:active{background:#e94560;color:#fff}
.transport button:disabled{opacity:0.3;cursor:not-allowed}
.progress-bar{width:100%;height:8px;background:#0f3460;border-radius:4px;overflow:hidden;margin:8px 0}
.progress-fill{height:100%;background:#e94560;width:0%;transition:width .3s}

/* Config */
.config-section{background:#16213e;border-radius:8px;padding:16px;margin:12px 0}
.config-section h3{color:#e94560;font-size:0.95em;margin-bottom:10px;
border-bottom:1px solid #0f3460;padding-bottom:6px}
.cfg-row{display:flex;justify-content:space-between;align-items:center;padding:6px 0;font-size:0.9em}
.cfg-row label{color:#a0a0a0;flex:1}
.cfg-row input[type=number],.cfg-row input[type=text],.cfg-row input[type=password]{
width:100px;padding:4px 8px;border:1px solid #0f3460;border-radius:4px;
background:#0a0a1a;color:#4ecca3;font-size:0.9em;text-align:right}
.cfg-row input[type=text],.cfg-row input[type=password]{width:160px;text-align:left}
.cfg-row input[type=checkbox]{accent-color:#e94560;width:18px;height:18px}
.notes-table{width:100%;border-collapse:collapse;font-size:0.8em;margin-top:8px}
.notes-table th{text-align:left;color:#e94560;padding:4px;border-bottom:1px solid #0f3460}
.notes-table td{padding:4px;border-bottom:1px solid #0a0a1a}
.notes-table input[type=number]{width:55px;padding:2px 4px;border:1px solid #0f3460;
border-radius:3px;background:#0a0a1a;color:#4ecca3;font-size:0.9em;text-align:right}
.notes-table .fingers{font-family:monospace;letter-spacing:2px}
.btn-row{display:flex;gap:8px;margin-top:16px}
.save-btn{flex:1;padding:12px;background:#4ecca3;color:#1a1a2e;border:none;border-radius:8px;
font-size:1em;font-weight:bold;cursor:pointer}
.save-btn:active{background:#3ab88a}
.reset-btn{flex:1;padding:12px;background:#0f3460;color:#e0e0e0;border:none;border-radius:8px;
font-size:1em;cursor:pointer}
.reset-btn:active{background:#e94560;color:#fff}
.cfg-status{text-align:center;margin:8px 0;font-size:0.85em;min-height:1.2em}
.cfg-status.ok{color:#4ecca3}.cfg-status.err{color:#e94560}

/* Monitor */
.monitor-grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin:12px 0}
.monitor-card{background:#16213e;border-radius:8px;padding:12px}
.monitor-card h4{color:#e94560;font-size:0.85em;margin-bottom:8px}
.monitor-card .big{font-size:1.8em;font-weight:bold;color:#4ecca3}
.cc-bar{display:flex;align-items:center;gap:8px;margin:4px 0}
.cc-bar .cc-label{min-width:50px;font-size:0.8em;color:#a0a0a0}
.cc-bar .cc-track{flex:1;height:6px;background:#0a0a1a;border-radius:3px;overflow:hidden}
.cc-bar .cc-fill{height:100%;background:#4ecca3;transition:width .2s}
.cc-bar .cc-val{min-width:30px;text-align:right;font-size:0.8em}
.log{background:#0a0a1a;border-radius:6px;padding:8px;max-height:150px;overflow-y:auto;
font-family:monospace;font-size:0.75em;line-height:1.6}
.log .entry{border-bottom:1px solid #16213e;padding:2px 0}

/* Panic */
.panic-btn{width:100%;padding:12px;margin-top:16px;background:#c0392b;color:#fff;
border:none;border-radius:8px;font-size:1em;font-weight:bold;cursor:pointer}
.panic-btn:active{background:#e74c3c}

@media(max-width:500px){
  .key{width:calc(33.33% - 6px);min-width:60px;padding:10px 4px}
  .monitor-grid{grid-template-columns:1fr}
  .cfg-row input[type=text],.cfg-row input[type=password]{width:120px}
}
</style>
</head>
<body>
<div class="header">
  <h1>ServoFlute</h1>
  <div class="status-bar">
    <span class="status-dot" id="sDot"></span>
    <span id="sText">Connexion...</span>
  </div>
</div>

<nav>
  <button class="active" onclick="showTab('keyboard')">Clavier</button>
  <button onclick="showTab('player')">Lecteur MIDI</button>
  <button onclick="showTab('config')">Config</button>
  <button onclick="showTab('monitor')">Monitor</button>
</nav>

<!-- TAB: CLAVIER VIRTUEL -->
<div class="tab active" id="tab-keyboard">
  <div class="slider-group">
    <label>Velocity</label>
    <input type="range" id="velSlider" min="1" max="127" value="100"
      oninput="setVelocity(this.value)">
    <span class="val" id="velVal">100</span>
  </div>

  <div class="keyboard" id="pianoKeys"></div>

  <div class="slider-group">
    <label>Volume CC7</label>
    <input type="range" id="cc7Slider" min="0" max="127" value="127"
      oninput="sendCC(7,this.value);document.getElementById('cc7Val').textContent=this.value">
    <span class="val" id="cc7Val">127</span>
  </div>
  <div class="slider-group">
    <label>Expression CC11</label>
    <input type="range" id="cc11Slider" min="0" max="127" value="127"
      oninput="sendCC(11,this.value);document.getElementById('cc11Val').textContent=this.value">
    <span class="val" id="cc11Val">127</span>
  </div>

  <button class="panic-btn" onclick="sendPanic()">ALL SOUND OFF</button>
</div>

<!-- TAB: LECTEUR MIDI -->
<div class="tab" id="tab-player">
  <div class="upload-zone" id="uploadZone" onclick="document.getElementById('midiFile').click()">
    <p>Glissez un fichier .mid ici</p>
    <p style="font-size:0.85em;color:#888;margin-top:8px">ou cliquez pour parcourir</p>
    <input type="file" id="midiFile" accept=".mid,.midi" onchange="uploadMidi(this.files[0])">
  </div>

  <div class="file-info" id="fileInfo">
    <div class="cfg-row"><span class="label" style="color:#a0a0a0">Fichier</span><span class="value" style="color:#4ecca3;font-weight:bold" id="fName">-</span></div>
    <div class="cfg-row"><span class="label" style="color:#a0a0a0">Evenements</span><span class="value" style="color:#4ecca3;font-weight:bold" id="fEvents">-</span></div>
    <div class="cfg-row"><span class="label" style="color:#a0a0a0">Duree</span><span class="value" style="color:#4ecca3;font-weight:bold" id="fDuration">-</span></div>
  </div>

  <div class="progress-bar"><div class="progress-fill" id="progressFill"></div></div>
  <div style="text-align:center;font-size:0.85em;color:#888" id="progressText">--:-- / --:--</div>

  <div class="transport">
    <button onclick="wsSend({t:'play'})" id="btnPlay" disabled>&#9654; Play</button>
    <button onclick="wsSend({t:'pause'})" id="btnPause" disabled>&#10074;&#10074; Pause</button>
    <button onclick="wsSend({t:'stop'})" id="btnStop" disabled>&#9632; Stop</button>
  </div>

  <button class="panic-btn" onclick="sendPanic()">ALL SOUND OFF</button>
</div>

<!-- TAB: CONFIGURATION (editable) -->
<div class="tab" id="tab-config">
  <div class="config-section">
    <h3>Instrument</h3>
    <div class="cfg-row"><label>Nom appareil</label><input type="text" id="cfgDevice" maxlength="31"></div>
    <div class="cfg-row"><label>Canal MIDI (0=omni)</label><input type="number" id="cfgMidiCh" min="0" max="16"></div>
  </div>
  <div class="config-section">
    <h3>Timing</h3>
    <div class="cfg-row"><label>Delai servo-solenoide (ms)</label><input type="number" id="cfgServoDelay" min="0" max="500"></div>
    <div class="cfg-row"><label>Interval min valve (ms)</label><input type="number" id="cfgValveInt" min="0" max="500"></div>
    <div class="cfg-row"><label>Duree min note (ms)</label><input type="number" id="cfgMinNote" min="0" max="500"></div>
  </div>
  <div class="config-section">
    <h3>Airflow</h3>
    <div class="cfg-row"><label>Angle repos (off)</label><input type="number" id="cfgAirOff" min="0" max="180"></div>
    <div class="cfg-row"><label>Angle min</label><input type="number" id="cfgAirMin" min="0" max="180"></div>
    <div class="cfg-row"><label>Angle max</label><input type="number" id="cfgAirMax" min="0" max="180"></div>
  </div>
  <div class="config-section">
    <h3>Vibrato</h3>
    <div class="cfg-row"><label>Frequence (Hz)</label><input type="number" id="cfgVibFreq" min="0.1" max="20" step="0.1"></div>
    <div class="cfg-row"><label>Amplitude max (deg)</label><input type="number" id="cfgVibAmp" min="0" max="30" step="0.5"></div>
  </div>
  <div class="config-section">
    <h3>Breath Controller (CC2)</h3>
    <div class="cfg-row"><label>Active</label><input type="checkbox" id="cfgCC2On"></div>
    <div class="cfg-row"><label>Seuil silence</label><input type="number" id="cfgCC2Thr" min="0" max="127"></div>
    <div class="cfg-row"><label>Courbe reponse</label><input type="number" id="cfgCC2Curve" min="0.1" max="5" step="0.1"></div>
    <div class="cfg-row"><label>Timeout (ms)</label><input type="number" id="cfgCC2Timeout" min="0" max="10000"></div>
  </div>
  <div class="config-section">
    <h3>Solenoide PWM</h3>
    <div class="cfg-row"><label>PWM activation</label><input type="number" id="cfgSolAct" min="0" max="255"></div>
    <div class="cfg-row"><label>PWM maintien</label><input type="number" id="cfgSolHold" min="0" max="255"></div>
    <div class="cfg-row"><label>Temps activation (ms)</label><input type="number" id="cfgSolTime" min="0" max="500"></div>
  </div>
  <div class="config-section">
    <h3>Calibration doigts</h3>
    <div class="cfg-row"><label>Angle ouverture</label><input type="number" id="cfgAngleOpen" min="10" max="90"></div>
    <table class="notes-table">
      <thead><tr><th>Doigt</th><th>PCA</th><th>Angle ferme</th><th>Direction</th></tr></thead>
      <tbody id="fingersTable"></tbody>
    </table>
  </div>
  <div class="config-section">
    <h3>Airflow par note (%)</h3>
    <table class="notes-table">
      <thead><tr><th>MIDI</th><th>Note</th><th>Air min%</th><th>Air max%</th></tr></thead>
      <tbody id="notesAirTable"></tbody>
    </table>
  </div>
  <div class="config-section">
    <h3>WiFi STA</h3>
    <div class="cfg-row"><label>SSID</label><input type="text" id="cfgWifiSsid" maxlength="32"></div>
    <div class="cfg-row"><label>Mot de passe</label><input type="password" id="cfgWifiPass" maxlength="64"></div>
  </div>
  <div class="config-section">
    <h3>Power</h3>
    <div class="cfg-row"><label>Timeout inactivite (ms)</label><input type="number" id="cfgTimeUnpower" min="0" max="60000"></div>
  </div>

  <div class="btn-row">
    <button class="save-btn" onclick="saveConfig()">Sauvegarder</button>
    <button class="reset-btn" onclick="resetConfig()">Reset defauts</button>
  </div>
  <div class="cfg-status" id="cfgStatus"></div>
</div>

<!-- TAB: MONITORING -->
<div class="tab" id="tab-monitor">
  <div class="monitor-grid">
    <div class="monitor-card">
      <h4>Etat</h4>
      <div class="big" id="monState">IDLE</div>
    </div>
    <div class="monitor-card">
      <h4>Heap libre</h4>
      <div class="big" id="monHeap">-</div>
    </div>
  </div>

  <div class="config-section">
    <h3>Control Changes</h3>
    <div class="cc-bar"><span class="cc-label">CC1 Mod</span>
      <div class="cc-track"><div class="cc-fill" id="ccBar1" style="width:0%"></div></div>
      <span class="cc-val" id="ccV1">0</span></div>
    <div class="cc-bar"><span class="cc-label">CC2 Breath</span>
      <div class="cc-track"><div class="cc-fill" id="ccBar2" style="width:0%"></div></div>
      <span class="cc-val" id="ccV2">127</span></div>
    <div class="cc-bar"><span class="cc-label">CC7 Vol</span>
      <div class="cc-track"><div class="cc-fill" id="ccBar7" style="width:0%"></div></div>
      <span class="cc-val" id="ccV7">127</span></div>
    <div class="cc-bar"><span class="cc-label">CC11 Expr</span>
      <div class="cc-track"><div class="cc-fill" id="ccBar11" style="width:0%"></div></div>
      <span class="cc-val" id="ccV11">127</span></div>
  </div>

  <div class="config-section">
    <h3>Journal</h3>
    <div class="log" id="logBox"></div>
  </div>

  <button class="panic-btn" onclick="sendPanic()">ALL SOUND OFF</button>
</div>

<script>
// Notes jouables de la flute
const NOTES=[
  {midi:82,name:"A#5",oct:"Basse",black:true},
  {midi:83,name:"B5",oct:"Basse",black:false},
  {midi:84,name:"C6",oct:"Medium",black:false},
  {midi:86,name:"D6",oct:"Medium",black:false},
  {midi:88,name:"E6",oct:"Medium",black:false},
  {midi:89,name:"F6",oct:"Medium",black:false},
  {midi:91,name:"G6",oct:"Medium",black:false},
  {midi:93,name:"A6",oct:"Medium",black:false},
  {midi:95,name:"B6",oct:"Medium",black:false},
  {midi:96,name:"C7",oct:"Aigu",black:false},
  {midi:98,name:"D7",oct:"Aigu",black:false},
  {midi:100,name:"E7",oct:"Aigu",black:false},
  {midi:101,name:"F7",oct:"Aigu",black:false},
  {midi:103,name:"G7",oct:"Aigu",black:false}
];

const STATES=["IDLE","POSITIONING","PLAYING","STOPPING"];
const NOTE_NAMES=['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'];
let ws=null, velocity=100, wsConnected=false;
let cfgData=null; // Derniere config chargee

// --- WebSocket ---
function wsConnect(){
  const proto=location.protocol==='https:'?'wss:':'ws:';
  ws=new WebSocket(proto+'//'+location.host+'/ws');
  ws.onopen=()=>{wsConnected=true;updateDot(true);addLog("WebSocket connecte")};
  ws.onclose=()=>{wsConnected=false;updateDot(false);setTimeout(wsConnect,2000)};
  ws.onerror=()=>{ws.close()};
  ws.onmessage=(e)=>{
    try{const d=JSON.parse(e.data);handleWsMsg(d)}catch(ex){}
  };
}

function wsSend(obj){if(ws&&ws.readyState===1)ws.send(JSON.stringify(obj))}

function handleWsMsg(d){
  if(d.t==='status'){
    const st=STATES[d.state]||"?";
    document.getElementById('monState').textContent=st;
    document.getElementById('monState').style.color=d.playing?'#e94560':'#4ecca3';
    if(d.heap)document.getElementById('monHeap').textContent=Math.round(d.heap/1024)+"KB";
    updateCC(1,d.cc1);updateCC(2,d.cc2);updateCC(7,d.cc7);updateCC(11,d.cc11);
    if(d.pp!==undefined){
      document.getElementById('progressFill').style.width=d.pp+'%';
      if(d.ppos!==undefined){
        document.getElementById('progressText').textContent=
          formatTime(d.ppos)+' / '+formatTime(playerDuration);
      }
    }
    const ps=d.ps;
    if(ps!==undefined){
      document.getElementById('btnPlay').disabled=(ps===1);
      document.getElementById('btnPause').disabled=(ps!==1);
      document.getElementById('btnStop').disabled=(ps===0&&!fileLoaded);
    }
  } else if(d.t==='midi_loaded'){
    fileLoaded=true;playerDuration=d.duration||0;
    document.getElementById('fileInfo').classList.add('visible');
    document.getElementById('fName').textContent=d.file;
    document.getElementById('fEvents').textContent=d.events;
    document.getElementById('fDuration').textContent=formatTime(d.duration);
    document.getElementById('btnPlay').disabled=false;
    document.getElementById('btnStop').disabled=false;
    addLog("MIDI charge: "+d.file+" ("+d.events+" evt)");
  } else if(d.t==='midi_error'){
    addLog("ERREUR MIDI: "+d.msg);
  }
}

function updateCC(num,val){
  if(val===undefined)return;
  const pct=(val/127*100).toFixed(0);
  const bar=document.getElementById('ccBar'+num);
  const txt=document.getElementById('ccV'+num);
  if(bar)bar.style.width=pct+'%';
  if(txt)txt.textContent=val;
}

function updateDot(on){
  document.getElementById('sDot').className='status-dot '+(on?'on':'off');
  document.getElementById('sText').textContent=on?'Connecte':'Deconnecte';
}

// --- Clavier virtuel ---
function buildKeyboard(){
  const container=document.getElementById('pianoKeys');
  let currentOct='';
  NOTES.forEach(n=>{
    if(n.oct!==currentOct){
      currentOct=n.oct;
      const lbl=document.createElement('div');
      lbl.className='octave-label';
      lbl.textContent='Octave '+currentOct;
      container.appendChild(lbl);
    }
    const key=document.createElement('div');
    key.className='key'+(n.black?' black':'');
    key.dataset.midi=n.midi;
    key.innerHTML='<span class="note-name">'+n.name+'</span><span class="note-midi">MIDI '+n.midi+'</span>';
    key.addEventListener('touchstart',(e)=>{e.preventDefault();noteOn(n.midi);key.classList.add('pressed')},{passive:false});
    key.addEventListener('touchend',(e)=>{e.preventDefault();noteOff(n.midi);key.classList.remove('pressed')},{passive:false});
    key.addEventListener('touchcancel',(e)=>{noteOff(n.midi);key.classList.remove('pressed')});
    key.addEventListener('mousedown',(e)=>{e.preventDefault();noteOn(n.midi);key.classList.add('pressed')});
    key.addEventListener('mouseup',(e)=>{noteOff(n.midi);key.classList.remove('pressed')});
    key.addEventListener('mouseleave',(e)=>{if(key.classList.contains('pressed')){noteOff(n.midi);key.classList.remove('pressed')}});
    container.appendChild(key);
  });
}

function noteOn(midi){wsSend({t:'non',n:midi,v:velocity});addLog("Note ON: "+midi+" vel="+velocity)}
function noteOff(midi){wsSend({t:'nof',n:midi});addLog("Note OFF: "+midi)}
function sendCC(num,val){wsSend({t:'cc',c:parseInt(num),v:parseInt(val)})}
function sendPanic(){wsSend({t:'panic'});addLog("ALL SOUND OFF")}
function setVelocity(v){velocity=parseInt(v);document.getElementById('velVal').textContent=v;
  wsSend({t:'velocity',v:velocity})}

// --- Player ---
let fileLoaded=false,playerDuration=0;

function uploadMidi(file){
  if(!file)return;
  addLog("Upload: "+file.name+" ("+file.size+" octets)");
  const fd=new FormData();
  fd.append('file',file,file.name);
  fetch('/api/midi',{method:'POST',body:fd})
    .then(r=>r.json())
    .then(d=>{
      if(d.ok){
        fileLoaded=true;playerDuration=d.duration||0;
        document.getElementById('fileInfo').classList.add('visible');
        document.getElementById('fName').textContent=d.file||file.name;
        document.getElementById('fEvents').textContent=d.events;
        document.getElementById('fDuration').textContent=formatTime(d.duration);
        document.getElementById('btnPlay').disabled=false;
        document.getElementById('btnStop').disabled=false;
        addLog("MIDI OK: "+d.events+" evenements");
      } else {
        addLog("ERREUR: "+(d.msg||"Echec upload"));
      }
    })
    .catch(e=>addLog("ERREUR upload: "+e));
}

const uz=document.getElementById('uploadZone');
uz.addEventListener('dragover',(e)=>{e.preventDefault();uz.classList.add('dragover')});
uz.addEventListener('dragleave',()=>{uz.classList.remove('dragover')});
uz.addEventListener('drop',(e)=>{
  e.preventDefault();uz.classList.remove('dragover');
  if(e.dataTransfer.files.length>0)uploadMidi(e.dataTransfer.files[0]);
});

// --- Tabs ---
function showTab(name){
  document.querySelectorAll('.tab').forEach(t=>t.classList.remove('active'));
  document.querySelectorAll('nav button').forEach(b=>b.classList.remove('active'));
  document.getElementById('tab-'+name).classList.add('active');
  event.target.classList.add('active');
  if(name==='config')loadConfig();
}

// --- Config ---
function loadConfig(){
  fetch('/api/config').then(r=>r.json()).then(d=>{
    cfgData=d;
    // Instrument
    document.getElementById('cfgDevice').value=d.device||'';
    document.getElementById('cfgMidiCh').value=d.midi_ch||0;
    // Timing
    document.getElementById('cfgServoDelay').value=d.servo_delay||0;
    document.getElementById('cfgValveInt').value=d.valve_interval||0;
    document.getElementById('cfgMinNote').value=d.min_note_dur||0;
    // Airflow
    document.getElementById('cfgAirOff').value=d.air_off||0;
    document.getElementById('cfgAirMin').value=d.air_min||0;
    document.getElementById('cfgAirMax').value=d.air_max||0;
    // Vibrato
    document.getElementById('cfgVibFreq').value=d.vib_freq||6;
    document.getElementById('cfgVibAmp').value=d.vib_amp||8;
    // CC2
    document.getElementById('cfgCC2On').checked=!!d.cc2_on;
    document.getElementById('cfgCC2Thr').value=d.cc2_thr||0;
    document.getElementById('cfgCC2Curve').value=d.cc2_curve||1.4;
    document.getElementById('cfgCC2Timeout').value=d.cc2_timeout||1000;
    // Solenoid
    document.getElementById('cfgSolAct').value=d.sol_act||255;
    document.getElementById('cfgSolHold').value=d.sol_hold||128;
    document.getElementById('cfgSolTime').value=d.sol_time||50;
    // Fingers
    document.getElementById('cfgAngleOpen').value=d.angle_open||30;
    const ft=document.getElementById('fingersTable');
    ft.innerHTML='';
    if(d.fingers){
      d.fingers.forEach((f,i)=>{
        const tr=document.createElement('tr');
        tr.innerHTML='<td>Doigt '+(i+1)+'</td><td>'+f.ch+'</td>'+
          '<td><input type="number" id="fAngle'+i+'" min="0" max="180" value="'+f.a+'"></td>'+
          '<td><select id="fDir'+i+'"><option value="1"'+(f.d===1?' selected':'')+'>+1</option>'+
          '<option value="-1"'+(f.d===-1?' selected':'')+'>-1</option></select></td>';
        ft.appendChild(tr);
      });
    }
    // Notes airflow
    const nt=document.getElementById('notesAirTable');
    nt.innerHTML='';
    if(d.notes){
      d.notes.forEach((n,i)=>{
        const name=NOTE_NAMES[n.midi%12]+(Math.floor(n.midi/12)-1);
        const tr=document.createElement('tr');
        tr.innerHTML='<td>'+n.midi+'</td><td>'+name+'</td>'+
          '<td><input type="number" id="nMin'+i+'" min="0" max="100" value="'+n.air_min+'"></td>'+
          '<td><input type="number" id="nMax'+i+'" min="0" max="100" value="'+n.air_max+'"></td>';
        nt.appendChild(tr);
      });
    }
    // WiFi
    document.getElementById('cfgWifiSsid').value=d.wifi_ssid||'';
    document.getElementById('cfgWifiPass').value='';
    // Power
    document.getElementById('cfgTimeUnpower').value=d.time_unpower||200;

    showCfgStatus('Config chargee','ok');
  }).catch(e=>{showCfgStatus('Erreur chargement: '+e,'err');addLog("Erreur config: "+e)});
}

function saveConfig(){
  const body={
    device:document.getElementById('cfgDevice').value,
    midi_ch:parseInt(document.getElementById('cfgMidiCh').value)||0,
    servo_delay:parseInt(document.getElementById('cfgServoDelay').value)||0,
    valve_interval:parseInt(document.getElementById('cfgValveInt').value)||0,
    min_note_dur:parseInt(document.getElementById('cfgMinNote').value)||0,
    air_off:parseInt(document.getElementById('cfgAirOff').value)||0,
    air_min:parseInt(document.getElementById('cfgAirMin').value)||0,
    air_max:parseInt(document.getElementById('cfgAirMax').value)||0,
    vib_freq:parseFloat(document.getElementById('cfgVibFreq').value)||6,
    vib_amp:parseFloat(document.getElementById('cfgVibAmp').value)||8,
    cc2_on:document.getElementById('cfgCC2On').checked,
    cc2_thr:parseInt(document.getElementById('cfgCC2Thr').value)||0,
    cc2_curve:parseFloat(document.getElementById('cfgCC2Curve').value)||1.4,
    cc2_timeout:parseInt(document.getElementById('cfgCC2Timeout').value)||1000,
    sol_act:parseInt(document.getElementById('cfgSolAct').value)||255,
    sol_hold:parseInt(document.getElementById('cfgSolHold').value)||128,
    sol_time:parseInt(document.getElementById('cfgSolTime').value)||50,
    angle_open:parseInt(document.getElementById('cfgAngleOpen').value)||30,
    time_unpower:parseInt(document.getElementById('cfgTimeUnpower').value)||200,
    wifi_ssid:document.getElementById('cfgWifiSsid').value
  };
  // Only send password if non-empty (avoid clearing saved password)
  const pass=document.getElementById('cfgWifiPass').value;
  if(pass.length>0)body.wifi_pass=pass;

  // Fingers
  if(cfgData&&cfgData.fingers){
    body.fingers=cfgData.fingers.map((f,i)=>({
      a:parseInt(document.getElementById('fAngle'+i).value)||f.a,
      d:parseInt(document.getElementById('fDir'+i).value)||f.d
    }));
  }
  // Notes airflow
  if(cfgData&&cfgData.notes){
    body.notes_air=cfgData.notes.map((n,i)=>({
      mn:parseInt(document.getElementById('nMin'+i).value)||0,
      mx:parseInt(document.getElementById('nMax'+i).value)||0
    }));
  }

  showCfgStatus('Sauvegarde...','ok');
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(body)})
    .then(r=>r.json())
    .then(d=>{
      if(d.ok){
        showCfgStatus('Config sauvegardee !','ok');
        addLog("Config sauvegardee");
      } else {
        showCfgStatus('Erreur: '+(d.msg||'echec'),'err');
      }
    })
    .catch(e=>showCfgStatus('Erreur: '+e,'err'));
}

function resetConfig(){
  if(!confirm('Remettre tous les parametres aux valeurs par defaut ?'))return;
  showCfgStatus('Reset...','ok');
  fetch('/api/config/reset',{method:'POST'})
    .then(r=>r.json())
    .then(d=>{
      if(d.ok){
        showCfgStatus('Reset OK - rechargement...','ok');
        addLog("Config reset aux defauts");
        setTimeout(loadConfig,500);
      } else {
        showCfgStatus('Erreur reset','err');
      }
    })
    .catch(e=>showCfgStatus('Erreur: '+e,'err'));
}

function showCfgStatus(msg,cls){
  const el=document.getElementById('cfgStatus');
  el.textContent=msg;el.className='cfg-status '+cls;
  if(cls==='ok')setTimeout(()=>{el.textContent=''},3000);
}

// --- Utils ---
function formatTime(ms){
  if(!ms||ms<=0)return"--:--";
  const s=Math.floor(ms/1000);
  const m=Math.floor(s/60);
  return String(m).padStart(2,'0')+':'+String(s%60).padStart(2,'0');
}

function addLog(msg){
  const box=document.getElementById('logBox');
  const entry=document.createElement('div');
  entry.className='entry';
  const now=new Date();
  const ts=String(now.getHours()).padStart(2,'0')+':'+String(now.getMinutes()).padStart(2,'0')+':'+String(now.getSeconds()).padStart(2,'0');
  entry.textContent='['+ts+'] '+msg;
  box.insertBefore(entry,box.firstChild);
  while(box.children.length>50)box.removeChild(box.lastChild);
}

// --- Clavier physique (raccourcis) ---
const KEY_MAP={'a':82,'z':83,'e':84,'r':86,'t':88,'y':89,'u':91,'i':93,'o':95,'p':96,
  'q':98,'s':100,'d':101,'f':103};
const keysDown=new Set();
document.addEventListener('keydown',(e)=>{
  if(e.target.tagName==='INPUT'||e.target.tagName==='SELECT')return;
  if(e.repeat)return;
  const note=KEY_MAP[e.key.toLowerCase()];
  if(note&&!keysDown.has(e.key)){
    keysDown.add(e.key);noteOn(note);
    const el=document.querySelector('.key[data-midi="'+note+'"]');
    if(el)el.classList.add('pressed');
  }
});
document.addEventListener('keyup',(e)=>{
  const note=KEY_MAP[e.key.toLowerCase()];
  if(note){
    keysDown.delete(e.key);noteOff(note);
    const el=document.querySelector('.key[data-midi="'+note+'"]');
    if(el)el.classList.remove('pressed');
  }
});

// --- Init ---
buildKeyboard();
wsConnect();
loadConfig();
</script>
</body>
</html>
)rawliteral";

#endif
