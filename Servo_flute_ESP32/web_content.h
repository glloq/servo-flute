/***********************************************************************************************
 * web_content.h - Interface web embarquee en PROGMEM
 *
 * Single Page Application avec 4 onglets :
 * 1. Clavier virtuel (14 notes jouables + velocity)
 * 2. Lecteur MIDI (upload + transport)
 * 3. Configuration (lecture des parametres)
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
.config-row{display:flex;justify-content:space-between;padding:6px 0;font-size:0.9em}
.config-row .label{color:#a0a0a0}.config-row .value{color:#4ecca3;font-weight:bold}
.notes-table{width:100%;border-collapse:collapse;font-size:0.8em;margin-top:8px}
.notes-table th{text-align:left;color:#e94560;padding:4px;border-bottom:1px solid #0f3460}
.notes-table td{padding:4px;border-bottom:1px solid #0a0a1a}
.notes-table .fingers{font-family:monospace;letter-spacing:2px}

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
    <div class="config-row"><span class="label">Fichier</span><span class="value" id="fName">-</span></div>
    <div class="config-row"><span class="label">Evenements</span><span class="value" id="fEvents">-</span></div>
    <div class="config-row"><span class="label">Duree</span><span class="value" id="fDuration">-</span></div>
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

<!-- TAB: CONFIGURATION -->
<div class="tab" id="tab-config">
  <div class="config-section">
    <h3>Instrument</h3>
    <div class="config-row"><span class="label">Nom</span><span class="value" id="cfgName">-</span></div>
    <div class="config-row"><span class="label">Canal MIDI</span><span class="value" id="cfgCh">-</span></div>
    <div class="config-row"><span class="label">Notes jouables</span><span class="value" id="cfgNotes">-</span></div>
    <div class="config-row"><span class="label">Servos doigts</span><span class="value" id="cfgFingers">-</span></div>
  </div>
  <div class="config-section">
    <h3>Airflow</h3>
    <div class="config-row"><span class="label">Angle min</span><span class="value" id="cfgAirMin">-</span></div>
    <div class="config-row"><span class="label">Angle max</span><span class="value" id="cfgAirMax">-</span></div>
    <div class="config-row"><span class="label">Delai servo-solenoide</span><span class="value" id="cfgDelay">-</span></div>
  </div>
  <div class="config-section">
    <h3>Vibrato</h3>
    <div class="config-row"><span class="label">Frequence</span><span class="value" id="cfgVibFreq">-</span></div>
    <div class="config-row"><span class="label">Amplitude max</span><span class="value" id="cfgVibAmp">-</span></div>
  </div>
  <div class="config-section">
    <h3>Breath Controller (CC2)</h3>
    <div class="config-row"><span class="label">Active</span><span class="value" id="cfgCC2">-</span></div>
    <div class="config-row"><span class="label">Seuil silence</span><span class="value" id="cfgCC2Thr">-</span></div>
    <div class="config-row"><span class="label">Courbe reponse</span><span class="value" id="cfgCC2Curve">-</span></div>
  </div>
  <div class="config-section">
    <h3>Notes et doigtes</h3>
    <table class="notes-table">
      <thead><tr><th>MIDI</th><th>Note</th><th>Doigtes</th><th>Air%</th></tr></thead>
      <tbody id="notesTableBody"></tbody>
    </table>
  </div>
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
let ws=null, velocity=100, wsConnected=false;

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
    // Update monitor
    const st=STATES[d.state]||"?";
    document.getElementById('monState').textContent=st;
    document.getElementById('monState').style.color=d.playing?'#e94560':'#4ecca3';
    if(d.heap)document.getElementById('monHeap').textContent=Math.round(d.heap/1024)+"KB";

    // CC bars
    updateCC(1,d.cc1);updateCC(2,d.cc2);updateCC(7,d.cc7);updateCC(11,d.cc11);

    // Player progress
    if(d.pp!==undefined){
      document.getElementById('progressFill').style.width=d.pp+'%';
      if(d.ppos!==undefined){
        document.getElementById('progressText').textContent=
          formatTime(d.ppos)+' / '+formatTime(playerDuration);
      }
    }
    // Player state buttons
    const ps=d.ps;
    if(ps!==undefined){
      document.getElementById('btnPlay').disabled=(ps===1);// playing
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

    // Touch events (mobile)
    key.addEventListener('touchstart',(e)=>{e.preventDefault();noteOn(n.midi);key.classList.add('pressed')},{passive:false});
    key.addEventListener('touchend',(e)=>{e.preventDefault();noteOff(n.midi);key.classList.remove('pressed')},{passive:false});
    key.addEventListener('touchcancel',(e)=>{noteOff(n.midi);key.classList.remove('pressed')});

    // Mouse events (desktop)
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

// Drag & drop
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
}

// --- Config ---
function loadConfig(){
  fetch('/api/config').then(r=>r.json()).then(d=>{
    document.getElementById('cfgName').textContent=d.device_name;
    document.getElementById('cfgCh').textContent=d.midi_channel===0?'Omni':d.midi_channel;
    document.getElementById('cfgNotes').textContent=d.num_notes;
    document.getElementById('cfgFingers').textContent=d.num_fingers;
    document.getElementById('cfgAirMin').textContent=d.airflow_min+'deg';
    document.getElementById('cfgAirMax').textContent=d.airflow_max+'deg';
    document.getElementById('cfgDelay').textContent=d.servo_solenoid_delay+'ms';
    document.getElementById('cfgVibFreq').textContent=d.vibrato_freq+'Hz';
    document.getElementById('cfgVibAmp').textContent=d.vibrato_amp+'deg';
    document.getElementById('cfgCC2').textContent=d.cc2_enabled?'Oui':'Non';
    document.getElementById('cfgCC2Thr').textContent=d.cc2_threshold;
    document.getElementById('cfgCC2Curve').textContent=d.cc2_curve;

    // Table des notes
    if(d.notes){
      const noteNames=['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'];
      const tbody=document.getElementById('notesTableBody');
      tbody.innerHTML='';
      d.notes.forEach(n=>{
        const name=noteNames[n.midi%12]+(Math.floor(n.midi/12)-1);
        const fingers=n.fingers.map(f=>f?'O':'-').join('');
        const tr=document.createElement('tr');
        tr.innerHTML='<td>'+n.midi+'</td><td>'+name+'</td><td class="fingers">'+fingers+'</td><td>'+n.air_min+'-'+n.air_max+'%</td>';
        tbody.appendChild(tr);
      });
    }
  }).catch(e=>addLog("Erreur config: "+e));
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
  // Limiter a 50 entrees
  while(box.children.length>50)box.removeChild(box.lastChild);
}

// --- Clavier physique (raccourcis) ---
const KEY_MAP={'a':82,'z':83,'e':84,'r':86,'t':88,'y':89,'u':91,'i':93,'o':95,'p':96,
  'q':98,'s':100,'d':101,'f':103};
const keysDown=new Set();
document.addEventListener('keydown',(e)=>{
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
