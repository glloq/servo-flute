/***********************************************************************************************
 * web_content.h - Interface web embarquee en PROGMEM
 *
 * Single Page Application avec 6 onglets :
 * 1. Clavier virtuel (dynamique depuis /api/config)
 * 2. Lecteur MIDI (upload + transport)
 * 3. Calibration (test servos doigts, airflow, solenoide)
 * 4. Config (parametres editables + sauvegarde JSON)
 * 5. WiFi (scan reseaux, connexion, statut)
 * 6. Monitoring temps reel (WebSocket)
 *
 * Interface responsive / mobile-friendly
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
background:#1a1a2e;color:#e0e0e0;overflow-x:hidden;min-height:100vh}
.header{background:#16213e;padding:10px 16px;display:flex;justify-content:space-between;
align-items:center;border-bottom:2px solid #0f3460;position:sticky;top:0;z-index:10}
.header h1{font-size:1.1em;color:#e94560}
.status-dot{width:10px;height:10px;border-radius:50%;display:inline-block;margin-right:6px}
.status-dot.on{background:#4ecca3}.status-dot.off{background:#e94560}
.status-bar{font-size:0.8em;color:#a0a0a0}
nav{display:flex;background:#16213e;border-bottom:1px solid #0f3460;
overflow-x:auto;-webkit-overflow-scrolling:touch;position:sticky;top:42px;z-index:10}
nav::-webkit-scrollbar{display:none}
nav button{flex:0 0 auto;padding:9px 12px;border:none;background:transparent;color:#a0a0a0;
font-size:0.8em;cursor:pointer;border-bottom:2px solid transparent;transition:all .2s;white-space:nowrap}
nav button.active{color:#e94560;border-bottom-color:#e94560;background:#1a1a2e}
nav button:hover{color:#fff}
.tab{display:none;padding:12px;max-width:800px;margin:0 auto}
.tab.active{display:block}

/* Clavier */
.keyboard{display:flex;flex-wrap:wrap;gap:5px;justify-content:center;margin:12px 0}
.key{width:calc(25% - 5px);min-width:65px;padding:12px 6px;border:2px solid #0f3460;
border-radius:8px;background:#16213e;color:#e0e0e0;font-size:0.85em;text-align:center;
cursor:pointer;user-select:none;-webkit-user-select:none;touch-action:none;transition:all .1s}
.key:active,.key.pressed{background:#e94560;border-color:#e94560;color:#fff;transform:scale(0.95)}
.key .note-name{font-weight:bold;font-size:1.05em;display:block}
.key .note-midi{font-size:0.7em;color:#888;display:block;margin-top:2px}
.key.black{background:#0a0a1a;border-color:#333}
.key.black:active,.key.black.pressed{background:#c0392b;border-color:#c0392b}
.octave-label{width:100%;text-align:center;font-size:0.75em;color:#666;margin:6px 0 3px}

/* Slider */
.slider-group{display:flex;align-items:center;gap:10px;margin:10px 0;padding:10px;
background:#16213e;border-radius:8px}
.slider-group label{min-width:70px;font-size:0.85em}
.slider-group input[type=range]{flex:1;accent-color:#e94560;min-width:0}
.slider-group .val{min-width:35px;text-align:right;font-weight:bold;color:#e94560;font-size:0.9em}

/* Player */
.upload-zone{border:2px dashed #0f3460;border-radius:12px;padding:24px;text-align:center;
margin:12px 0;cursor:pointer;transition:all .2s}
.upload-zone:hover,.upload-zone.dragover{border-color:#e94560;background:rgba(233,69,96,0.1)}
.upload-zone input{display:none}
.file-info{background:#16213e;border-radius:8px;padding:10px;margin:10px 0;display:none}
.file-info.visible{display:block}
.transport{display:flex;gap:8px;justify-content:center;margin:12px 0}
.transport button{padding:10px 20px;border:2px solid #0f3460;border-radius:8px;
background:#16213e;color:#e0e0e0;font-size:0.95em;cursor:pointer;transition:all .2s}
.transport button:hover{border-color:#e94560;color:#e94560}
.transport button:active{background:#e94560;color:#fff}
.transport button:disabled{opacity:0.3;cursor:not-allowed}
.progress-bar{width:100%;height:8px;background:#0f3460;border-radius:4px;overflow:hidden;margin:8px 0}
.progress-fill{height:100%;background:#e94560;width:0%;transition:width .3s}

/* Config / sections */
.section{background:#16213e;border-radius:8px;padding:14px;margin:10px 0}
.section h3{color:#e94560;font-size:0.9em;margin-bottom:8px;
border-bottom:1px solid #0f3460;padding-bottom:5px}
.cfg-row{display:flex;justify-content:space-between;align-items:center;padding:5px 0;font-size:0.85em}
.cfg-row label{color:#a0a0a0;flex:1;min-width:0}
.cfg-row input[type=number],.cfg-row input[type=text],.cfg-row input[type=password]{
width:90px;padding:4px 6px;border:1px solid #0f3460;border-radius:4px;
background:#0a0a1a;color:#4ecca3;font-size:0.85em;text-align:right}
.cfg-row input[type=text],.cfg-row input[type=password]{width:140px;text-align:left}
.cfg-row input[type=checkbox]{accent-color:#e94560;width:18px;height:18px}
.cfg-row select{padding:4px 6px;border:1px solid #0f3460;border-radius:4px;
background:#0a0a1a;color:#4ecca3;font-size:0.85em}

/* Tables */
.tbl{width:100%;border-collapse:collapse;font-size:0.8em;margin-top:6px}
.tbl th{text-align:left;color:#e94560;padding:3px 4px;border-bottom:1px solid #0f3460}
.tbl td{padding:3px 4px;border-bottom:1px solid #0a0a1a}
.tbl input[type=number]{width:52px;padding:2px 4px;border:1px solid #0f3460;
border-radius:3px;background:#0a0a1a;color:#4ecca3;font-size:0.85em;text-align:right}
.tbl select{padding:2px 4px;border:1px solid #0f3460;border-radius:3px;
background:#0a0a1a;color:#4ecca3;font-size:0.85em}

/* Buttons */
.btn-row{display:flex;gap:8px;margin-top:14px}
.btn-save{flex:1;padding:11px;background:#4ecca3;color:#1a1a2e;border:none;border-radius:8px;
font-size:0.95em;font-weight:bold;cursor:pointer}
.btn-save:active{background:#3ab88a}
.btn-secondary{flex:1;padding:11px;background:#0f3460;color:#e0e0e0;border:none;border-radius:8px;
font-size:0.95em;cursor:pointer}
.btn-secondary:active{background:#e94560;color:#fff}
.btn-action{padding:8px 16px;background:#0f3460;color:#e0e0e0;border:none;border-radius:6px;
font-size:0.85em;cursor:pointer;transition:all .2s}
.btn-action:hover{background:#e94560;color:#fff}
.btn-action:disabled{opacity:0.4;cursor:not-allowed}
.cfg-status{text-align:center;margin:6px 0;font-size:0.8em;min-height:1.1em}
.cfg-status.ok{color:#4ecca3}.cfg-status.err{color:#e94560}

/* Calibration */
.cal-servo{background:#0a0a1a;border-radius:6px;padding:10px;margin:8px 0}
.cal-servo .cal-label{font-size:0.85em;color:#a0a0a0;margin-bottom:4px}
.cal-servo .cal-val{font-size:1.3em;font-weight:bold;color:#4ecca3;text-align:center}
.cal-range{width:100%;accent-color:#e94560}

/* WiFi */
.wifi-list{max-height:250px;overflow-y:auto;margin:8px 0}
.wifi-item{display:flex;justify-content:space-between;align-items:center;padding:10px;
background:#0a0a1a;border-radius:6px;margin:4px 0;cursor:pointer;transition:all .2s}
.wifi-item:hover{background:#0f3460}
.wifi-item.selected{border:1px solid #e94560;background:#16213e}
.wifi-ssid{font-weight:bold;font-size:0.9em}
.wifi-meta{font-size:0.75em;color:#888;display:flex;gap:10px;align-items:center}
.signal-bars{display:inline-flex;gap:1px;align-items:flex-end;height:12px}
.signal-bars span{width:3px;background:#4ecca3;border-radius:1px}
.wifi-status{padding:10px;background:#0a0a1a;border-radius:6px;margin:8px 0;font-size:0.85em}

/* Monitor */
.monitor-grid{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin:10px 0}
.monitor-card{background:#16213e;border-radius:8px;padding:10px}
.monitor-card h4{color:#e94560;font-size:0.8em;margin-bottom:6px}
.monitor-card .big{font-size:1.6em;font-weight:bold;color:#4ecca3}
.cc-bar{display:flex;align-items:center;gap:6px;margin:3px 0}
.cc-bar .cc-label{min-width:48px;font-size:0.75em;color:#a0a0a0}
.cc-bar .cc-track{flex:1;height:5px;background:#0a0a1a;border-radius:3px;overflow:hidden}
.cc-bar .cc-fill{height:100%;background:#4ecca3;transition:width .2s}
.cc-bar .cc-val{min-width:28px;text-align:right;font-size:0.75em}
.log{background:#0a0a1a;border-radius:6px;padding:6px;max-height:130px;overflow-y:auto;
font-family:monospace;font-size:0.7em;line-height:1.5}
.log .entry{border-bottom:1px solid #16213e;padding:1px 0}

/* Panic */
.panic-btn{width:100%;padding:11px;margin-top:14px;background:#c0392b;color:#fff;
border:none;border-radius:8px;font-size:0.95em;font-weight:bold;cursor:pointer}
.panic-btn:active{background:#e74c3c}

@media(max-width:500px){
  .key{width:calc(33.33% - 5px);min-width:55px;padding:9px 4px;font-size:0.8em}
  .monitor-grid{grid-template-columns:1fr}
  .cfg-row{flex-wrap:wrap;gap:4px}
  .cfg-row input[type=text],.cfg-row input[type=password]{width:100%;text-align:left}
  .header h1{font-size:1em}
  nav button{padding:8px 10px;font-size:0.75em}
  .slider-group label{min-width:55px;font-size:0.8em}
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

<nav id="navBar">
  <button class="active" onclick="showTab('keyboard',this)">Clavier</button>
  <button onclick="showTab('player',this)">MIDI</button>
  <button onclick="showTab('calibration',this)">Calibration</button>
  <button onclick="showTab('config',this)">Config</button>
  <button onclick="showTab('wifi',this)">WiFi</button>
  <button onclick="showTab('monitor',this)">Monitor</button>
</nav>

<!-- TAB: CLAVIER VIRTUEL (dynamique) -->
<div class="tab active" id="tab-keyboard">
  <div class="slider-group">
    <label>Velocity</label>
    <input type="range" id="velSlider" min="1" max="127" value="100"
      oninput="setVelocity(this.value)">
    <span class="val" id="velVal">100</span>
  </div>

  <div class="keyboard" id="pianoKeys">
    <div style="text-align:center;color:#888;padding:20px">Chargement des notes...</div>
  </div>

  <div class="slider-group">
    <label>Vol CC7</label>
    <input type="range" id="cc7Slider" min="0" max="127" value="127"
      oninput="sendCC(7,this.value);document.getElementById('cc7Val').textContent=this.value">
    <span class="val" id="cc7Val">127</span>
  </div>
  <div class="slider-group">
    <label>Expr CC11</label>
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
    <p style="font-size:0.8em;color:#888;margin-top:6px">ou cliquez pour parcourir</p>
    <input type="file" id="midiFile" accept=".mid,.midi" onchange="uploadMidi(this.files[0])">
  </div>

  <div class="file-info" id="fileInfo">
    <div class="cfg-row"><span style="color:#a0a0a0">Fichier</span><span style="color:#4ecca3;font-weight:bold" id="fName">-</span></div>
    <div class="cfg-row"><span style="color:#a0a0a0">Evenements</span><span style="color:#4ecca3;font-weight:bold" id="fEvents">-</span></div>
    <div class="cfg-row"><span style="color:#a0a0a0">Duree</span><span style="color:#4ecca3;font-weight:bold" id="fDuration">-</span></div>
  </div>

  <div class="progress-bar"><div class="progress-fill" id="progressFill"></div></div>
  <div style="text-align:center;font-size:0.8em;color:#888" id="progressText">--:-- / --:--</div>

  <div class="transport">
    <button onclick="wsSend({t:'play'})" id="btnPlay" disabled>&#9654; Play</button>
    <button onclick="wsSend({t:'pause'})" id="btnPause" disabled>&#10074;&#10074;</button>
    <button onclick="wsSend({t:'stop'})" id="btnStop" disabled>&#9632; Stop</button>
  </div>

  <button class="panic-btn" onclick="sendPanic()">ALL SOUND OFF</button>
</div>

<!-- TAB: CALIBRATION -->
<div class="tab" id="tab-calibration">
  <div class="section">
    <h3>Test servos doigts</h3>
    <div id="calFingers">Chargement...</div>
  </div>

  <div class="section">
    <h3>Test airflow</h3>
    <div class="cal-servo">
      <div class="cal-label">Servo airflow</div>
      <div class="cal-val" id="calAirVal">20</div>
      <input type="range" class="cal-range" id="calAirSlider" min="0" max="180" value="20"
        oninput="testAirflow(this.value)">
    </div>
  </div>

  <div class="section">
    <h3>Test solenoide</h3>
    <div style="display:flex;gap:8px">
      <button class="btn-action" style="flex:1" onclick="testSolenoid(1)">OUVRIR</button>
      <button class="btn-action" style="flex:1" onclick="testSolenoid(0)">FERMER</button>
    </div>
  </div>

  <div class="section">
    <h3>Test note complete</h3>
    <div class="cfg-row">
      <label>Note MIDI</label>
      <select id="calNoteSelect"></select>
      <button class="btn-action" onclick="testNote()" style="margin-left:8px">Jouer position</button>
    </div>
    <p style="font-size:0.75em;color:#888;margin-top:6px">
      Positionne les doigts + airflow pour la note (sans solenoide).
    </p>
  </div>

  <button class="panic-btn" onclick="sendPanic();resetCalibration()">TOUT ARRETER</button>
</div>

<!-- TAB: CONFIGURATION -->
<div class="tab" id="tab-config">
  <div class="section">
    <h3>Instrument</h3>
    <div class="cfg-row"><label>Nom appareil</label><input type="text" id="cfgDevice" maxlength="31"></div>
    <div class="cfg-row"><label>Canal MIDI (0=omni)</label><input type="number" id="cfgMidiCh" min="0" max="16"></div>
  </div>
  <div class="section">
    <h3>Timing</h3>
    <div class="cfg-row"><label>Delai servo-sol. (ms)</label><input type="number" id="cfgServoDelay" min="0" max="500"></div>
    <div class="cfg-row"><label>Interval valve (ms)</label><input type="number" id="cfgValveInt" min="0" max="500"></div>
    <div class="cfg-row"><label>Duree min note (ms)</label><input type="number" id="cfgMinNote" min="0" max="500"></div>
  </div>
  <div class="section">
    <h3>Airflow</h3>
    <div class="cfg-row"><label>Angle repos</label><input type="number" id="cfgAirOff" min="0" max="180"></div>
    <div class="cfg-row"><label>Angle min</label><input type="number" id="cfgAirMin" min="0" max="180"></div>
    <div class="cfg-row"><label>Angle max</label><input type="number" id="cfgAirMax" min="0" max="180"></div>
  </div>
  <div class="section">
    <h3>Vibrato</h3>
    <div class="cfg-row"><label>Frequence (Hz)</label><input type="number" id="cfgVibFreq" min="0.1" max="20" step="0.1"></div>
    <div class="cfg-row"><label>Amplitude (deg)</label><input type="number" id="cfgVibAmp" min="0" max="30" step="0.5"></div>
  </div>
  <div class="section">
    <h3>Breath CC2</h3>
    <div class="cfg-row"><label>Active</label><input type="checkbox" id="cfgCC2On"></div>
    <div class="cfg-row"><label>Seuil silence</label><input type="number" id="cfgCC2Thr" min="0" max="127"></div>
    <div class="cfg-row"><label>Courbe reponse</label><input type="number" id="cfgCC2Curve" min="0.1" max="5" step="0.1"></div>
    <div class="cfg-row"><label>Timeout (ms)</label><input type="number" id="cfgCC2Timeout" min="0" max="10000"></div>
  </div>
  <div class="section">
    <h3>Solenoide PWM</h3>
    <div class="cfg-row"><label>PWM activation</label><input type="number" id="cfgSolAct" min="0" max="255"></div>
    <div class="cfg-row"><label>PWM maintien</label><input type="number" id="cfgSolHold" min="0" max="255"></div>
    <div class="cfg-row"><label>Temps activation (ms)</label><input type="number" id="cfgSolTime" min="0" max="500"></div>
  </div>
  <div class="section">
    <h3>Doigts</h3>
    <div class="cfg-row"><label>Angle ouverture</label><input type="number" id="cfgAngleOpen" min="10" max="90"></div>
    <table class="tbl">
      <thead><tr><th>Doigt</th><th>PCA</th><th>Angle ferme</th><th>Dir</th></tr></thead>
      <tbody id="fingersTable"></tbody>
    </table>
  </div>
  <div class="section">
    <h3>Airflow par note</h3>
    <table class="tbl">
      <thead><tr><th>MIDI</th><th>Note</th><th>Air min%</th><th>Air max%</th></tr></thead>
      <tbody id="notesAirTable"></tbody>
    </table>
  </div>
  <div class="section">
    <h3>Power</h3>
    <div class="cfg-row"><label>Timeout inactivite (ms)</label><input type="number" id="cfgTimeUnpower" min="0" max="60000"></div>
  </div>

  <div class="btn-row">
    <button class="btn-save" onclick="saveConfig()">Sauvegarder</button>
    <button class="btn-secondary" onclick="resetConfig()">Reset defauts</button>
  </div>
  <div class="cfg-status" id="cfgStatus"></div>
</div>

<!-- TAB: WIFI -->
<div class="tab" id="tab-wifi">
  <div class="section">
    <h3>Etat actuel</h3>
    <div class="wifi-status" id="wifiStatus">Chargement...</div>
  </div>

  <div class="section">
    <h3>Reseaux disponibles</h3>
    <div style="display:flex;gap:8px;margin-bottom:8px">
      <button class="btn-action" id="btnScan" onclick="startWifiScan()">Scanner</button>
      <span style="font-size:0.8em;color:#888;align-self:center" id="scanStatus"></span>
    </div>
    <div class="wifi-list" id="wifiList"></div>
  </div>

  <div class="section">
    <h3>Connexion</h3>
    <div class="cfg-row"><label>SSID</label><input type="text" id="wifiSsid" maxlength="32" placeholder="Nom du reseau"></div>
    <div class="cfg-row"><label>Mot de passe</label><input type="password" id="wifiPass" maxlength="64" placeholder=""></div>
    <div class="btn-row" style="margin-top:10px">
      <button class="btn-save" onclick="connectWifi()">Connecter</button>
    </div>
    <div class="cfg-status" id="wifiConnStatus"></div>
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

  <div class="section">
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

  <div class="section">
    <h3>Journal</h3>
    <div class="log" id="logBox"></div>
  </div>

  <button class="panic-btn" onclick="sendPanic()">ALL SOUND OFF</button>
</div>

<script>
const NOTE_NAMES=['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'];
const STATES=["IDLE","POSITIONING","PLAYING","STOPPING"];
let ws=null,velocity=100,wsConnected=false;
let cfgData=null,notesData=[],numFingers=6;

// --- Helpers ---
function midiToName(m){return NOTE_NAMES[m%12]+(Math.floor(m/12)-1)}
function isBlackKey(m){return[1,3,6,8,10].includes(m%12)}
function formatTime(ms){if(!ms||ms<=0)return"--:--";const s=Math.floor(ms/1000);
  return String(Math.floor(s/60)).padStart(2,'0')+':'+String(s%60).padStart(2,'0')}
function $(id){return document.getElementById(id)}

// --- WebSocket ---
function wsConnect(){
  const proto=location.protocol==='https:'?'wss:':'ws:';
  ws=new WebSocket(proto+'//'+location.host+'/ws');
  ws.onopen=()=>{wsConnected=true;updateDot(true);addLog("WS connecte")};
  ws.onclose=()=>{wsConnected=false;updateDot(false);setTimeout(wsConnect,2000)};
  ws.onerror=()=>{ws.close()};
  ws.onmessage=(e)=>{try{handleWsMsg(JSON.parse(e.data))}catch(ex){}};
}
function wsSend(obj){if(ws&&ws.readyState===1)ws.send(JSON.stringify(obj))}

function handleWsMsg(d){
  if(d.t==='status'){
    const st=STATES[d.state]||"?";
    $('monState').textContent=st;
    $('monState').style.color=d.playing?'#e94560':'#4ecca3';
    if(d.heap)$('monHeap').textContent=Math.round(d.heap/1024)+"KB";
    updateCC(1,d.cc1);updateCC(2,d.cc2);updateCC(7,d.cc7);updateCC(11,d.cc11);
    if(d.pp!==undefined){
      $('progressFill').style.width=d.pp+'%';
      if(d.ppos!==undefined)$('progressText').textContent=formatTime(d.ppos)+' / '+formatTime(playerDuration);
    }
    if(d.ps!==undefined){
      $('btnPlay').disabled=(d.ps===1);
      $('btnPause').disabled=(d.ps!==1);
      $('btnStop').disabled=(d.ps===0&&!fileLoaded);
    }
  }else if(d.t==='midi_loaded'){
    fileLoaded=true;playerDuration=d.duration||0;
    $('fileInfo').classList.add('visible');
    $('fName').textContent=d.file;$('fEvents').textContent=d.events;
    $('fDuration').textContent=formatTime(d.duration);
    $('btnPlay').disabled=false;$('btnStop').disabled=false;
    addLog("MIDI: "+d.file+" ("+d.events+" evt)");
  }else if(d.t==='midi_error'){addLog("ERR MIDI: "+d.msg)}
}

function updateCC(num,val){
  if(val===undefined)return;const pct=(val/127*100).toFixed(0);
  const bar=$('ccBar'+num),txt=$('ccV'+num);
  if(bar)bar.style.width=pct+'%';if(txt)txt.textContent=val;
}
function updateDot(on){
  $('sDot').className='status-dot '+(on?'on':'off');
  $('sText').textContent=on?'Connecte':'Deconnecte';
}

// --- Clavier dynamique ---
function buildKeyboard(){
  const c=$('pianoKeys');c.innerHTML='';
  if(!notesData.length){c.innerHTML='<div style="color:#888;padding:16px;text-align:center">Aucune note configuree</div>';return}
  let prevOct='';
  // Grouper par octave
  notesData.forEach(n=>{
    const name=midiToName(n.midi);
    const oct=name.replace(/[^0-9]/g,'');
    if(oct!==prevOct){
      prevOct=oct;
      const lbl=document.createElement('div');lbl.className='octave-label';
      lbl.textContent='Octave '+oct;c.appendChild(lbl);
    }
    const key=document.createElement('div');
    key.className='key'+(isBlackKey(n.midi)?' black':'');
    key.dataset.midi=n.midi;
    key.innerHTML='<span class="note-name">'+name+'</span><span class="note-midi">MIDI '+n.midi+'</span>';
    key.addEventListener('touchstart',(e)=>{e.preventDefault();noteOn(n.midi);key.classList.add('pressed')},{passive:false});
    key.addEventListener('touchend',(e)=>{e.preventDefault();noteOff(n.midi);key.classList.remove('pressed')},{passive:false});
    key.addEventListener('touchcancel',()=>{noteOff(n.midi);key.classList.remove('pressed')});
    key.addEventListener('mousedown',(e)=>{e.preventDefault();noteOn(n.midi);key.classList.add('pressed')});
    key.addEventListener('mouseup',()=>{noteOff(n.midi);key.classList.remove('pressed')});
    key.addEventListener('mouseleave',()=>{if(key.classList.contains('pressed')){noteOff(n.midi);key.classList.remove('pressed')}});
    c.appendChild(key);
  });
  buildKeyMap();
}

function noteOn(midi){wsSend({t:'non',n:midi,v:velocity});addLog("ON: "+midiToName(midi)+" ("+midi+")")}
function noteOff(midi){wsSend({t:'nof',n:midi})}
function sendCC(num,val){wsSend({t:'cc',c:parseInt(num),v:parseInt(val)})}
function sendPanic(){wsSend({t:'panic'});addLog("ALL SOUND OFF")}
function setVelocity(v){velocity=parseInt(v);$('velVal').textContent=v;wsSend({t:'velocity',v:velocity})}

// Clavier physique dynamique
const KEY_CHARS='azertyuiopqsdfghjklmwxcvbn'.split('');
let keyMap={},keysDown=new Set();

function buildKeyMap(){
  keyMap={};
  notesData.forEach((n,i)=>{if(i<KEY_CHARS.length)keyMap[KEY_CHARS[i]]=n.midi});
}

document.addEventListener('keydown',(e)=>{
  if(e.target.tagName==='INPUT'||e.target.tagName==='SELECT'||e.target.tagName==='TEXTAREA')return;
  if(e.repeat)return;
  const note=keyMap[e.key.toLowerCase()];
  if(note&&!keysDown.has(e.key)){
    keysDown.add(e.key);noteOn(note);
    const el=document.querySelector('.key[data-midi="'+note+'"]');
    if(el)el.classList.add('pressed');
  }
});
document.addEventListener('keyup',(e)=>{
  const note=keyMap[e.key.toLowerCase()];
  if(note){keysDown.delete(e.key);noteOff(note);
    const el=document.querySelector('.key[data-midi="'+note+'"]');
    if(el)el.classList.remove('pressed');
  }
});

// --- Calibration ---
function buildCalibration(){
  const c=$('calFingers');c.innerHTML='';
  for(let i=0;i<numFingers;i++){
    const d=document.createElement('div');d.className='cal-servo';
    const closed=cfgData?cfgData.fingers[i].a:90;
    d.innerHTML='<div class="cal-label">Doigt '+(i+1)+' (PCA '+(cfgData?cfgData.fingers[i].ch:i)+')</div>'+
      '<div class="cal-val" id="calF'+i+'Val">'+closed+'</div>'+
      '<input type="range" class="cal-range" id="calF'+i+'" min="0" max="180" value="'+closed+
      '" oninput="testFinger('+i+',this.value)">';
    c.appendChild(d);
  }
  // Note test select
  const sel=$('calNoteSelect');sel.innerHTML='';
  notesData.forEach(n=>{
    const o=document.createElement('option');o.value=n.midi;
    o.textContent=midiToName(n.midi)+' (MIDI '+n.midi+')';sel.appendChild(o);
  });
  // Init airflow slider
  if(cfgData){$('calAirSlider').value=cfgData.air_off;$('calAirVal').textContent=cfgData.air_off}
}

function testFinger(idx,angle){
  $('calF'+idx+'Val').textContent=angle;
  wsSend({t:'test_finger',i:idx,a:parseInt(angle)});
}
function testAirflow(angle){
  $('calAirVal').textContent=angle;
  wsSend({t:'test_air',a:parseInt(angle)});
}
function testSolenoid(open){wsSend({t:'test_sol',o:open})}
function testNote(){
  const midi=parseInt($('calNoteSelect').value);
  if(midi)wsSend({t:'test_note',n:midi});
}
function resetCalibration(){
  // Remettre airflow au repos
  if(cfgData){testAirflow(cfgData.air_off);$('calAirSlider').value=cfgData.air_off}
  testSolenoid(0);
}

// --- Config ---
function loadConfig(){
  fetch('/api/config').then(r=>r.json()).then(d=>{
    cfgData=d;
    numFingers=d.num_fingers||6;
    notesData=d.notes||[];
    // Remplir formulaires
    $('cfgDevice').value=d.device||'';
    $('cfgMidiCh').value=d.midi_ch||0;
    $('cfgServoDelay').value=d.servo_delay||0;
    $('cfgValveInt').value=d.valve_interval||0;
    $('cfgMinNote').value=d.min_note_dur||0;
    $('cfgAirOff').value=d.air_off||0;
    $('cfgAirMin').value=d.air_min||0;
    $('cfgAirMax').value=d.air_max||0;
    $('cfgVibFreq').value=d.vib_freq||6;
    $('cfgVibAmp').value=d.vib_amp||8;
    $('cfgCC2On').checked=!!d.cc2_on;
    $('cfgCC2Thr').value=d.cc2_thr||0;
    $('cfgCC2Curve').value=d.cc2_curve||1.4;
    $('cfgCC2Timeout').value=d.cc2_timeout||1000;
    $('cfgSolAct').value=d.sol_act||255;
    $('cfgSolHold').value=d.sol_hold||128;
    $('cfgSolTime').value=d.sol_time||50;
    $('cfgAngleOpen').value=d.angle_open||30;
    $('cfgTimeUnpower').value=d.time_unpower||200;
    // Fingers table
    const ft=$('fingersTable');ft.innerHTML='';
    if(d.fingers){d.fingers.forEach((f,i)=>{
      const tr=document.createElement('tr');
      tr.innerHTML='<td>'+(i+1)+'</td><td>'+f.ch+'</td>'+
        '<td><input type="number" id="fAngle'+i+'" min="0" max="180" value="'+f.a+'"></td>'+
        '<td><select id="fDir'+i+'"><option value="1"'+(f.d===1?' selected':'')+'>+1</option>'+
        '<option value="-1"'+(f.d===-1?' selected':'')+'>-1</option></select></td>';
      ft.appendChild(tr);
    })}
    // Notes airflow table
    const nt=$('notesAirTable');nt.innerHTML='';
    if(d.notes){d.notes.forEach((n,i)=>{
      const tr=document.createElement('tr');
      tr.innerHTML='<td>'+n.midi+'</td><td>'+midiToName(n.midi)+'</td>'+
        '<td><input type="number" id="nMin'+i+'" min="0" max="100" value="'+n.air_min+'"></td>'+
        '<td><input type="number" id="nMax'+i+'" min="0" max="100" value="'+n.air_max+'"></td>';
      nt.appendChild(tr);
    })}
    // Construire clavier et calibration dynamiquement
    buildKeyboard();
    buildCalibration();
    showCfgStatus('Config chargee','ok');
  }).catch(e=>{showCfgStatus('Erreur: '+e,'err')});
}

function saveConfig(){
  const body={
    device:$('cfgDevice').value,
    midi_ch:parseInt($('cfgMidiCh').value)||0,
    servo_delay:parseInt($('cfgServoDelay').value)||0,
    valve_interval:parseInt($('cfgValveInt').value)||0,
    min_note_dur:parseInt($('cfgMinNote').value)||0,
    air_off:parseInt($('cfgAirOff').value)||0,
    air_min:parseInt($('cfgAirMin').value)||0,
    air_max:parseInt($('cfgAirMax').value)||0,
    vib_freq:parseFloat($('cfgVibFreq').value)||6,
    vib_amp:parseFloat($('cfgVibAmp').value)||8,
    cc2_on:$('cfgCC2On').checked,
    cc2_thr:parseInt($('cfgCC2Thr').value)||0,
    cc2_curve:parseFloat($('cfgCC2Curve').value)||1.4,
    cc2_timeout:parseInt($('cfgCC2Timeout').value)||1000,
    sol_act:parseInt($('cfgSolAct').value)||255,
    sol_hold:parseInt($('cfgSolHold').value)||128,
    sol_time:parseInt($('cfgSolTime').value)||50,
    angle_open:parseInt($('cfgAngleOpen').value)||30,
    time_unpower:parseInt($('cfgTimeUnpower').value)||200
  };
  if(cfgData&&cfgData.fingers){
    body.fingers=cfgData.fingers.map((f,i)=>({
      a:parseInt($('fAngle'+i).value)||f.a,
      d:parseInt($('fDir'+i).value)||f.d
    }));
  }
  if(cfgData&&cfgData.notes){
    body.notes_air=cfgData.notes.map((n,i)=>({
      mn:parseInt($('nMin'+i).value)||0,
      mx:parseInt($('nMax'+i).value)||0
    }));
  }
  showCfgStatus('Sauvegarde...','ok');
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(body)}).then(r=>r.json()).then(d=>{
    if(d.ok){showCfgStatus('Sauvegarde OK !','ok');addLog("Config sauvegardee")}
    else showCfgStatus('Erreur: '+(d.msg||''),'err');
  }).catch(e=>showCfgStatus('Erreur: '+e,'err'));
}

function resetConfig(){
  if(!confirm('Remettre les parametres par defaut ?'))return;
  fetch('/api/config/reset',{method:'POST'}).then(r=>r.json()).then(d=>{
    if(d.ok){showCfgStatus('Reset OK','ok');setTimeout(loadConfig,500)}
    else showCfgStatus('Erreur','err');
  }).catch(e=>showCfgStatus('Erreur: '+e,'err'));
}

function showCfgStatus(msg,cls){
  const el=$('cfgStatus');el.textContent=msg;el.className='cfg-status '+cls;
  if(cls==='ok')setTimeout(()=>{el.textContent=''},3000);
}

// --- WiFi ---
let scanTimer=null;

function loadWifiStatus(){
  fetch('/api/wifi/status').then(r=>r.json()).then(d=>{
    let html='';
    if(d.ap)html='<strong style="color:#e94560">Mode Hotspot (AP)</strong><br>IP: '+d.ip;
    else if(d.state===2)html='<strong style="color:#4ecca3">Connecte</strong><br>SSID: '+d.ssid+'<br>IP: '+d.ip+'<br>Signal: '+d.rssi+' dBm';
    else if(d.state===1)html='<strong style="color:#e9a645">Connexion en cours...</strong>';
    else html='<strong style="color:#888">Deconnecte</strong>';
    $('wifiStatus').innerHTML=html;
  }).catch(()=>{$('wifiStatus').textContent='Erreur'});
}

function startWifiScan(){
  $('btnScan').disabled=true;$('scanStatus').textContent='Scan en cours...';
  $('wifiList').innerHTML='';
  fetch('/api/wifi/scan').then(r=>r.json()).then(()=>{
    scanTimer=setInterval(pollScanResults,1500);
  });
}

function pollScanResults(){
  fetch('/api/wifi/results').then(r=>r.json()).then(d=>{
    if(d.done){
      clearInterval(scanTimer);scanTimer=null;
      $('btnScan').disabled=false;
      $('scanStatus').textContent=d.networks?d.networks.length+' reseaux':'0 reseau';
      renderWifiList(d.networks||[]);
    }
  });
}

function renderWifiList(nets){
  const c=$('wifiList');c.innerHTML='';
  // Trier par signal
  nets.sort((a,b)=>b.rssi-a.rssi);
  nets.forEach(n=>{
    const div=document.createElement('div');div.className='wifi-item';
    const bars=signalBars(n.rssi);
    div.innerHTML='<div><div class="wifi-ssid">'+escHtml(n.ssid)+'</div>'+
      '<div class="wifi-meta">'+bars+(n.enc?'&#128274;':'Ouvert')+'</div></div>'+
      '<div style="color:#888;font-size:0.75em">'+n.rssi+' dBm</div>';
    div.onclick=()=>{
      document.querySelectorAll('.wifi-item').forEach(i=>i.classList.remove('selected'));
      div.classList.add('selected');
      $('wifiSsid').value=n.ssid;
      $('wifiPass').value='';$('wifiPass').focus();
    };
    c.appendChild(div);
  });
}

function signalBars(rssi){
  let level=0;
  if(rssi>=-50)level=4;else if(rssi>=-60)level=3;else if(rssi>=-70)level=2;else level=1;
  let html='<span class="signal-bars">';
  for(let i=1;i<=4;i++){
    const h=3+i*2;
    html+='<span style="height:'+h+'px;opacity:'+(i<=level?1:0.2)+'"></span>';
  }
  return html+'</span>';
}

function connectWifi(){
  const ssid=$('wifiSsid').value.trim();
  if(!ssid){$('wifiConnStatus').textContent='SSID requis';$('wifiConnStatus').className='cfg-status err';return}
  const pass=$('wifiPass').value;
  $('wifiConnStatus').textContent='Connexion...';$('wifiConnStatus').className='cfg-status ok';
  fetch('/api/wifi/connect',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({ssid:ssid,pass:pass})}).then(r=>r.json()).then(d=>{
    if(d.ok){
      $('wifiConnStatus').textContent='Connexion lancee. La page peut devenir inaccessible si l\'IP change.';
      $('wifiConnStatus').className='cfg-status ok';
      addLog("WiFi: connexion vers "+ssid);
      // Recharger status apres delai
      setTimeout(loadWifiStatus,8000);
    }else{
      $('wifiConnStatus').textContent='Erreur: '+(d.msg||'');
      $('wifiConnStatus').className='cfg-status err';
    }
  }).catch(e=>{$('wifiConnStatus').textContent='Erreur: '+e;$('wifiConnStatus').className='cfg-status err'});
}

function escHtml(s){const d=document.createElement('div');d.textContent=s;return d.innerHTML}

// --- Player ---
let fileLoaded=false,playerDuration=0;

function uploadMidi(file){
  if(!file)return;addLog("Upload: "+file.name);
  const fd=new FormData();fd.append('file',file,file.name);
  fetch('/api/midi',{method:'POST',body:fd}).then(r=>r.json()).then(d=>{
    if(d.ok){
      fileLoaded=true;playerDuration=d.duration||0;
      $('fileInfo').classList.add('visible');
      $('fName').textContent=d.file||file.name;$('fEvents').textContent=d.events;
      $('fDuration').textContent=formatTime(d.duration);
      $('btnPlay').disabled=false;$('btnStop').disabled=false;
      addLog("MIDI OK: "+d.events+" evt");
    }else addLog("ERR: "+(d.msg||"echec"));
  }).catch(e=>addLog("ERR upload: "+e));
}

const uz=$('uploadZone');
uz.addEventListener('dragover',(e)=>{e.preventDefault();uz.classList.add('dragover')});
uz.addEventListener('dragleave',()=>{uz.classList.remove('dragover')});
uz.addEventListener('drop',(e)=>{e.preventDefault();uz.classList.remove('dragover');
  if(e.dataTransfer.files.length>0)uploadMidi(e.dataTransfer.files[0])});

// --- Tabs ---
function showTab(name,btn){
  document.querySelectorAll('.tab').forEach(t=>t.classList.remove('active'));
  document.querySelectorAll('nav button').forEach(b=>b.classList.remove('active'));
  $('tab-'+name).classList.add('active');
  if(btn)btn.classList.add('active');
  if(name==='config')loadConfig();
  if(name==='wifi'){loadWifiStatus()}
  if(name==='calibration'&&cfgData)buildCalibration();
}

// --- Log ---
function addLog(msg){
  const box=$('logBox');if(!box)return;
  const entry=document.createElement('div');entry.className='entry';
  const now=new Date();
  const ts=String(now.getHours()).padStart(2,'0')+':'+String(now.getMinutes()).padStart(2,'0')+':'+String(now.getSeconds()).padStart(2,'0');
  entry.textContent='['+ts+'] '+msg;
  box.insertBefore(entry,box.firstChild);
  while(box.children.length>50)box.removeChild(box.lastChild);
}

// --- Init ---
wsConnect();
loadConfig(); // Charge config + construit clavier + calibration
</script>
</body>
</html>
)rawliteral";

#endif
