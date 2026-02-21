/***********************************************************************************************
 * web_content.h - Interface web embarquee en PROGMEM
 *
 * Single Page Application avec 3 onglets + overlay settings :
 * 1. Clavier virtuel (dynamique depuis cfg.notes[])
 * 2. Lecteur MIDI (upload + transport)
 * 3. Calibration (3 etapes : Doigts, Doigtes, Souffle)
 * + Overlay Settings (engrenage) : tous les parametres avances
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
<title>ServoFlute</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;
background:#1a1a2e;color:#e0e0e0;overflow-x:hidden;min-height:100vh}
.hdr{background:#16213e;padding:10px 16px;display:flex;justify-content:space-between;
align-items:center;border-bottom:2px solid #0f3460;position:sticky;top:0;z-index:10}
.hdr h1{font-size:1.1em;color:#e94560}
.hdr-r{display:flex;align-items:center;gap:12px}
.dot{width:10px;height:10px;border-radius:50%;display:inline-block}
.dot.on{background:#4ecca3;box-shadow:0 0 6px #4ecca3}.dot.off{background:#555}
.gear-btn{background:none;border:none;color:#888;font-size:1.4em;cursor:pointer;padding:4px}
.gear-btn:hover{color:#e94560}
.tabs{display:flex;background:#16213e;border-bottom:1px solid #0f3460;overflow-x:auto}
.tabs button{flex:1;background:none;border:none;color:#888;padding:12px 8px;font-size:0.85em;
cursor:pointer;border-bottom:2px solid transparent;white-space:nowrap;min-width:80px}
.tabs button.active{color:#e94560;border-bottom-color:#e94560}
.tab{display:none;padding:12px}.tab.active{display:block}
.section{background:#16213e;border-radius:8px;padding:12px;margin-bottom:12px;border:1px solid #0f3460}
.section h3{color:#e94560;font-size:0.9em;margin-bottom:8px}
/* Buttons */
.btn{padding:8px 16px;border:none;border-radius:6px;cursor:pointer;font-size:0.85em}
.btn-p{background:#e94560;color:#fff}.btn-p:hover{background:#d63650}
.btn-s{background:#0f3460;color:#e0e0e0;border:1px solid #1a4080}.btn-s:hover{background:#1a4080}
.btn-g{background:#4ecca3;color:#1a1a2e}.btn-g:hover{background:#3db892}
.btn:disabled{opacity:.4;cursor:default}
.btn-row{display:flex;gap:8px;margin-top:10px;flex-wrap:wrap}
/* Inputs */
input[type=range]{width:100%;accent-color:#e94560}
input[type=number],input[type=text],input[type=password],select{background:#0d1b3e;border:1px solid #1a4080;
color:#e0e0e0;padding:6px 8px;border-radius:4px;font-size:0.85em;width:100%}
.cfg-row{display:flex;align-items:center;gap:8px;margin-bottom:6px}
.cfg-row label{flex:0 0 140px;font-size:0.8em;color:#aaa;text-align:right}
.cfg-row input,.cfg-row select{flex:1}
/* Keyboard */
.keys{display:flex;flex-wrap:wrap;gap:6px;justify-content:center;padding:8px 0}
.key{background:linear-gradient(180deg,#2a2a4a,#1a1a2e);border:1px solid #0f3460;
border-radius:6px;padding:10px 8px;text-align:center;cursor:pointer;user-select:none;
min-width:60px;flex:0 0 auto;transition:background .1s}
.key.black{background:linear-gradient(180deg,#1a1a2e,#0a0a1e);border-color:#333}
.key.pressed,.key:active{background:#e94560;border-color:#e94560}
.note-name{display:block;font-weight:bold;font-size:1em;color:#fff}
.note-midi{display:block;font-size:0.65em;color:#888;margin-top:2px}
.kf-row{display:flex;gap:3px;justify-content:center;margin-top:4px}
.kf{width:8px;height:8px;border-radius:50%;border:1px solid #555}
.kf.c{background:#333}.kf.o{background:#4ecca3}
/* SVG Flute */
.flute-box{background:#0d1b3e;border-radius:8px;padding:12px;text-align:center;margin-bottom:8px}
.flute-box svg{width:100%;max-width:600px;height:auto}
.flute-hole{stroke:#5C4A0A;stroke-width:2}
.flute-hole.closed{fill:#3a2a0a}.flute-hole.open{fill:#4ecca3}
.flute-hole.thumb{filter:drop-shadow(0 0 3px #e94560)}
.flute-lbl{font-size:11px;fill:#888}
.flute-num{font-size:10px;fill:#fff;font-weight:bold;pointer-events:none}
.flute-info{text-align:center;font-size:0.8em;color:#888;margin-top:4px}
/* Calibration steps */
.steps{display:flex;align-items:center;justify-content:center;gap:0;padding:12px 0}
.step-dot{width:12px;height:12px;border-radius:50%;background:#333;cursor:pointer;transition:.2s}
.step-dot.active{background:#e94560;box-shadow:0 0 8px #e94560}.step-dot.done{background:#4ecca3}
.step-line{width:40px;height:2px;background:#333}
.step-labels{display:flex;justify-content:center;gap:34px;font-size:0.75em;color:#888;margin-bottom:8px}
.cal-card{background:#0d1b3e;border:1px solid #1a4080;border-radius:8px;padding:10px;margin-bottom:8px}
.cal-card h4{font-size:0.85em;color:#e94560;margin-bottom:6px}
/* Fingering table */
.fg-row{display:flex;align-items:center;gap:8px;padding:6px 4px;border-bottom:1px solid #0f3460}
.fg-row:last-child{border-bottom:none}
.fg-note{font-weight:bold;min-width:50px;font-size:0.9em}
.fg-midi{color:#888;font-size:0.75em;min-width:36px}
.fg-dots{display:flex;gap:4px;flex:1}
.fg-dot{width:18px;height:18px;border-radius:50%;border:2px solid #555;cursor:pointer;transition:.15s}
.fg-dot.closed{background:#333}.fg-dot.open{background:#4ecca3;border-color:#4ecca3}
.fg-dot.thumb{border-style:dashed;border-color:#e94560}
/* Airflow per note */
.air-card{display:flex;align-items:center;gap:8px;padding:6px 0;border-bottom:1px solid #0f3460;flex-wrap:wrap}
.air-note{font-weight:bold;min-width:40px;font-size:0.85em}
.air-sliders{flex:1;min-width:150px}
.air-vals{font-size:0.75em;color:#888;display:flex;justify-content:space-between}
/* Settings overlay */
.settings-overlay{display:none;position:fixed;top:0;left:0;right:0;bottom:0;
background:rgba(0,0,0,.85);z-index:100;overflow-y:auto}
.settings-overlay.open{display:block}
.settings-box{max-width:600px;margin:0 auto;padding:16px}
.settings-box h2{color:#e94560;margin-bottom:12px;display:flex;justify-content:space-between;align-items:center}
.close-btn{background:none;border:none;color:#888;font-size:1.5em;cursor:pointer}
.close-btn:hover{color:#e94560}
/* MIDI player */
.drop-zone{border:2px dashed #0f3460;border-radius:8px;padding:30px;text-align:center;
color:#555;cursor:pointer;transition:border-color .2s}
.drop-zone.hover{border-color:#e94560;color:#e94560}
.transport{display:flex;gap:8px;justify-content:center;align-items:center;margin:12px 0}
.transport button{width:44px;height:44px;border-radius:50%;font-size:1.2em}
.progress-bar{height:6px;background:#0f3460;border-radius:3px;overflow:hidden;margin:8px 0}
.progress-fill{height:100%;background:#e94560;width:0%;transition:width .3s}
.file-info{font-size:0.8em;color:#888;text-align:center}
/* Monitor */
.cc-bar{display:flex;align-items:center;gap:8px;margin-bottom:6px;font-size:0.8em}
.cc-label{min-width:70px;color:#888}.cc-val{min-width:24px;text-align:right}
.cc-track{flex:1;height:6px;background:#0f3460;border-radius:3px;overflow:hidden}
.cc-fill{height:100%;background:#4ecca3;transition:width .2s}
/* VU meter */
.vu{display:flex;align-items:center;gap:8px;margin:8px 0}
.vu-track{flex:1;height:10px;background:#0f3460;border-radius:5px;overflow:hidden}
.vu-fill{height:100%;background:#4ecca3;width:0%;transition:width .1s}
.vu-val{font-size:0.8em;min-width:36px;text-align:right}
/* MIC badge */
.mic-badge{display:inline-block;background:#4ecca3;color:#1a1a2e;font-size:0.7em;
padding:2px 8px;border-radius:10px;font-weight:bold}
.mic-badge.off{background:#555;color:#888}
/* Pitch display */
.pitch{display:flex;gap:16px;align-items:center;font-size:0.9em}
.pitch-note{font-size:1.4em;font-weight:bold;color:#e94560;min-width:50px}
.pitch-hz{color:#888;font-size:0.85em}
.pitch-cents{font-size:0.85em}.pitch-cents.ok{color:#4ecca3}.pitch-cents.sharp{color:#e94560}.pitch-cents.flat{color:#e9a645}
/* Auto-cal progress */
.acal-progress{background:#0d1b3e;border-radius:8px;padding:12px;display:none}
.acal-bar{height:8px;background:#0f3460;border-radius:4px;overflow:hidden;margin:6px 0}
.acal-fill{height:100%;background:#e94560;width:0%;transition:width .3s}
.acal-info{font-size:0.8em;color:#888;display:flex;justify-content:space-between}
/* WiFi */
.wifi-item{padding:8px;border-bottom:1px solid #0f3460;cursor:pointer;display:flex;justify-content:space-between}
.wifi-item:hover{background:#0f3460}
/* Status bar */
.status-bar{background:#0d1117;padding:6px 16px;font-size:0.75em;color:#555;
display:flex;justify-content:space-between;position:fixed;bottom:0;left:0;right:0;z-index:5}
.log{background:#0a0a1a;border-radius:4px;padding:8px;font-family:monospace;font-size:0.75em;
max-height:120px;overflow-y:auto;color:#888}
</style>
</head>
<body>
<div class="hdr">
  <h1 id="devName">ServoFlute</h1>
  <div class="hdr-r">
    <span class="dot off" id="sDot"></span>
    <button class="gear-btn" onclick="toggleSettings()" title="Reglages">&#9881;</button>
  </div>
</div>

<div class="tabs">
  <button class="active" onclick="showTab('keyboard',this)">Clavier</button>
  <button onclick="showTab('midi',this)">MIDI</button>
  <button onclick="showTab('calib',this)">Calibration</button>
</div>

<!-- TAB: KEYBOARD -->
<div class="tab active" id="tab-keyboard">
  <div class="flute-box">
    <svg id="fluteSvg" viewBox="0 0 400 100"></svg>
    <div class="flute-info"><span id="fluteNote">-</span> <span id="fluteInfo" style="color:#555">Jouez une note</span></div>
  </div>
  <div class="section">
    <div class="cfg-row"><label>Velocity</label>
      <input type="range" min="1" max="127" value="100" id="velSlider" oninput="setVelocity(this.value)">
      <span id="velVal" style="min-width:28px;text-align:right">100</span>
    </div>
  </div>
  <div class="keys" id="pianoKeys"></div>
</div>

<!-- TAB: MIDI PLAYER -->
<div class="tab" id="tab-midi">
  <div class="section">
    <h3>Fichier MIDI</h3>
    <div class="drop-zone" id="dropZone" onclick="document.getElementById('midiFile').click()">
      Glisser-deposer un fichier MIDI ou cliquer
      <input type="file" id="midiFile" accept=".mid,.midi" style="display:none" onchange="uploadMidi(this)">
    </div>
    <div class="file-info" id="fileInfo" style="margin-top:8px">
      <span id="fName"></span> &bull; <span id="fEvents"></span> evt &bull; <span id="fDuration"></span>
    </div>
  </div>
  <div class="section">
    <div class="transport">
      <button class="btn btn-g" id="btnPlay" onclick="wsSend({t:'play'})" disabled>&#9654;</button>
      <button class="btn btn-s" id="btnPause" onclick="wsSend({t:'pause'})" disabled>&#10074;&#10074;</button>
      <button class="btn btn-p" id="btnStop" onclick="wsSend({t:'stop'})" disabled>&#9632;</button>
    </div>
    <div class="progress-bar"><div class="progress-fill" id="progressFill"></div></div>
    <div class="file-info" id="progressText">--:-- / --:--</div>
  </div>
</div>

<!-- TAB: CALIBRATION -->
<div class="tab" id="tab-calib">
  <div class="steps">
    <div class="step-dot active" onclick="goStep(1)"></div>
    <div class="step-line"></div>
    <div class="step-dot" onclick="goStep(2)"></div>
    <div class="step-line"></div>
    <div class="step-dot" onclick="goStep(3)"></div>
  </div>
  <div class="step-labels">
    <span>Doigts</span><span>Doigtes</span><span>Souffle</span>
  </div>

  <!-- STEP 1: FINGERS -->
  <div id="step1">
    <div class="section">
      <h3>Configuration des servos</h3>
      <div class="cfg-row"><label>Nombre de doigts</label>
        <div style="display:flex;align-items:center;gap:8px">
          <button class="btn btn-s" onclick="changeFingers(-1)">-</button>
          <span id="numFingersDisp" style="min-width:24px;text-align:center;font-weight:bold">6</span>
          <button class="btn btn-s" onclick="changeFingers(1)">+</button>
        </div>
      </div>
      <div class="cfg-row"><label>Servo airflow PCA</label>
        <select id="airPca" style="max-width:80px"></select>
      </div>
      <div class="cfg-row"><label>Amplitude ouverture</label>
        <input type="range" min="10" max="90" value="30" id="angleOpen" oninput="$('aoVal').textContent=this.value+'&deg;'">
        <span id="aoVal" style="min-width:36px">30&deg;</span>
      </div>
    </div>
    <div class="flute-box">
      <svg id="calFluteSvg" viewBox="0 0 400 100"></svg>
    </div>
    <div id="fingerCards"></div>
    <div class="btn-row" style="justify-content:flex-end">
      <button class="btn btn-p" onclick="saveStep1()">Sauver &amp; Continuer &rarr;</button>
    </div>
  </div>

  <!-- STEP 2: FINGERINGS -->
  <div id="step2" style="display:none">
    <div class="section">
      <h3>Doigtes par note</h3>
      <div class="cfg-row"><label>Preset</label>
        <select id="presetSelect" onchange="applyPreset(this.value)">
          <option value="">-- Personnalise --</option>
          <option value="irish_c">Flute irlandaise C</option>
        </select>
      </div>
    </div>
    <div class="section" id="fingeringSection">
      <div id="fingeringRows"></div>
      <div class="btn-row">
        <button class="btn btn-s" onclick="addNote()">+ Ajouter note</button>
        <button class="btn btn-s" onclick="removeLastNote()">- Supprimer</button>
      </div>
    </div>
    <div class="btn-row" style="justify-content:space-between">
      <button class="btn btn-s" onclick="goStep(1)">&larr; Retour</button>
      <button class="btn btn-p" onclick="saveStep2()">Sauver &amp; Continuer &rarr;</button>
    </div>
  </div>

  <!-- STEP 3: AIRFLOW -->
  <div id="step3" style="display:none">
    <div class="section" id="micSection" style="display:none">
      <h3><span class="mic-badge" id="micBadge">MIC</span> Auto-calibration</h3>
      <div class="vu"><span style="font-size:.8em;min-width:24px">VU</span>
        <div class="vu-track"><div class="vu-fill" id="vuFill"></div></div>
        <span class="vu-val" id="vuVal">0%</span>
      </div>
      <div class="pitch">
        <span class="pitch-note" id="pitchNote">-</span>
        <span class="pitch-hz" id="pitchHz">- Hz</span>
        <span class="pitch-cents ok" id="pitchCents">-</span>
      </div>
      <div class="btn-row">
        <button class="btn btn-g" id="btnAcalStart" onclick="startAutoCal()">Auto-calibrer toutes les notes</button>
        <button class="btn btn-p" id="btnAcalStop" onclick="stopAutoCal()" style="display:none">Arreter</button>
      </div>
      <div class="acal-progress" id="acalProgress">
        <div class="acal-info"><span id="acalStep">-</span><span id="acalState">-</span></div>
        <div class="acal-bar"><div class="acal-fill" id="acalFill"></div></div>
        <div id="acalResults" style="margin-top:8px;display:none"></div>
      </div>
    </div>
    <div class="section">
      <h3>Souffle par note</h3>
      <div id="airflowRows"></div>
    </div>
    <div class="btn-row" style="justify-content:space-between">
      <button class="btn btn-s" onclick="goStep(2)">&larr; Retour</button>
      <button class="btn btn-g" onclick="saveStep3()">Sauver &amp; Terminer &#10003;</button>
    </div>
  </div>
</div>

<!-- SETTINGS OVERLAY -->
<div class="settings-overlay" id="settingsOverlay">
<div class="settings-box">
  <h2>Reglages <button class="close-btn" onclick="toggleSettings()">&times;</button></h2>

  <div class="section"><h3>Appareil</h3>
    <div class="cfg-row"><label>Nom</label><input type="text" id="cfgDevice" maxlength="31"></div>
    <div class="cfg-row"><label>Canal MIDI</label><select id="cfgMidiCh">
      <option value="0">Omni (tous)</option>
    </select></div>
  </div>

  <div class="section"><h3>Timing</h3>
    <div class="cfg-row"><label>Servo&rarr;valve (ms)</label><input type="number" id="cfgDelay" min="0" max="1000"></div>
    <div class="cfg-row"><label>Intervalle valve (ms)</label><input type="number" id="cfgValveInt" min="0" max="500"></div>
    <div class="cfg-row"><label>Note min (ms)</label><input type="number" id="cfgMinNote" min="0" max="500"></div>
  </div>

  <div class="section"><h3>Servo Airflow</h3>
    <div class="cfg-row"><label>Angle repos</label><input type="number" id="cfgAirOff" min="0" max="180"></div>
    <div class="cfg-row"><label>Angle min</label><input type="number" id="cfgAirMin" min="0" max="180"></div>
    <div class="cfg-row"><label>Angle max</label><input type="number" id="cfgAirMax" min="0" max="180"></div>
  </div>

  <div class="section"><h3>Vibrato</h3>
    <div class="cfg-row"><label>Frequence (Hz)</label><input type="number" id="cfgVibF" min="0" max="20" step="0.5"></div>
    <div class="cfg-row"><label>Amplitude (deg)</label><input type="number" id="cfgVibA" min="0" max="30" step="0.5"></div>
  </div>

  <div class="section"><h3>Breath CC2</h3>
    <div class="cfg-row"><label>Active</label><input type="checkbox" id="cfgCC2On" style="width:auto;flex:0"></div>
    <div class="cfg-row"><label>Seuil silence</label><input type="number" id="cfgCC2Thr" min="0" max="127"></div>
    <div class="cfg-row"><label>Courbe</label><input type="number" id="cfgCC2Curve" min="0.1" max="5" step="0.1"></div>
    <div class="cfg-row"><label>Timeout (ms)</label><input type="number" id="cfgCC2To" min="0" max="10000"></div>
  </div>

  <div class="section"><h3>Solenoide PWM</h3>
    <div class="cfg-row"><label>PWM activation</label><input type="number" id="cfgSolAct" min="0" max="255"></div>
    <div class="cfg-row"><label>PWM maintien</label><input type="number" id="cfgSolHold" min="0" max="255"></div>
    <div class="cfg-row"><label>Temps (ms)</label><input type="number" id="cfgSolTime" min="0" max="500"></div>
  </div>

  <div class="section"><h3>Power</h3>
    <div class="cfg-row"><label>Timeout (ms)</label><input type="number" id="cfgUnpower" min="0" max="60000"></div>
  </div>

  <div class="section"><h3>WiFi</h3>
    <div class="cfg-row"><label>Etat</label><span id="wifiState" style="font-size:.85em;color:#888">-</span></div>
    <div class="btn-row"><button class="btn btn-s" onclick="startWifiScan()">Scanner</button>
      <span style="font-size:.75em;color:#555" id="scanStatus"></span></div>
    <div id="wifiList" style="margin:8px 0"></div>
    <div class="cfg-row"><label>SSID</label><input type="text" id="wifiSsid" maxlength="32"></div>
    <div class="cfg-row"><label>Mot de passe</label><input type="password" id="wifiPass" maxlength="64"></div>
    <div class="btn-row"><button class="btn btn-g" onclick="connectWifi()">Connecter</button></div>
    <div style="font-size:.75em;color:#888;margin-top:4px" id="wifiMsg"></div>
  </div>

  <div class="section"><h3>Monitor</h3>
    <div style="display:flex;gap:16px;margin-bottom:8px">
      <div><span style="color:#888;font-size:.8em">Etat</span><div style="font-weight:bold" id="monState">IDLE</div></div>
      <div><span style="color:#888;font-size:.8em">Heap</span><div style="font-weight:bold" id="monHeap">-</div></div>
    </div>
    <div class="cc-bar"><span class="cc-label">CC1 Mod</span><div class="cc-track"><div class="cc-fill" id="ccBar1"></div></div><span class="cc-val" id="ccV1">0</span></div>
    <div class="cc-bar"><span class="cc-label">CC2 Breath</span><div class="cc-track"><div class="cc-fill" id="ccBar2"></div></div><span class="cc-val" id="ccV2">127</span></div>
    <div class="cc-bar"><span class="cc-label">CC7 Vol</span><div class="cc-track"><div class="cc-fill" id="ccBar7"></div></div><span class="cc-val" id="ccV7">127</span></div>
    <div class="cc-bar"><span class="cc-label">CC11 Expr</span><div class="cc-track"><div class="cc-fill" id="ccBar11"></div></div><span class="cc-val" id="ccV11">127</span></div>
    <div class="log" id="logBox"></div>
    <div class="btn-row"><button class="btn btn-p" onclick="wsSend({t:'panic'})">ALL SOUND OFF</button></div>
  </div>

  <div class="btn-row" style="justify-content:center;margin-top:16px">
    <button class="btn btn-g" onclick="saveSettings()">Sauvegarder</button>
    <button class="btn btn-s" onclick="resetConfig()">Reset defauts</button>
  </div>
  <div style="font-size:.75em;color:#888;text-align:center;margin-top:8px" id="settingsMsg"></div>
</div>
</div>

<div class="status-bar">
  <span id="sText">Deconnecte</span>
  <span id="heapBar">-</span>
</div>

<script>
const N=['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'];
const STATES=['IDLE','POSITIONING','PLAYING','STOPPING'];
let ws=null,velocity=100,CFG=null,curNote=null;
let calibStep=1,fileLoaded=false,playerDuration=0;
let micDetected=false,autoCalRunning=false;

function mn(m){return N[m%12]+(Math.floor(m/12)-1)}
function isBlack(m){return[1,3,6,8,10].includes(m%12)}
function $(id){return document.getElementById(id)}
function fmt(ms){if(!ms||ms<=0)return'--:--';const s=Math.floor(ms/1000);return String(s/60|0).padStart(2,'0')+':'+String(s%60).padStart(2,'0')}
function addLog(t){const b=$('logBox');if(!b)return;b.innerHTML+='<div>'+t+'</div>';b.scrollTop=b.scrollHeight}

// --- Tabs ---
function showTab(id,btn){
  document.querySelectorAll('.tab').forEach(t=>t.classList.remove('active'));
  document.querySelectorAll('.tabs button').forEach(b=>b.classList.remove('active'));
  $('tab-'+id).classList.add('active');if(btn)btn.classList.add('active');
  if(id==='calib'&&CFG)buildCalibUI();
}
function toggleSettings(){$('settingsOverlay').classList.toggle('open');if($('settingsOverlay').classList.contains('open')&&CFG)fillSettings()}

// --- WebSocket ---
function wsConnect(){
  const p=location.protocol==='https:'?'wss:':'ws:';
  ws=new WebSocket(p+'//'+location.host+'/ws');
  ws.onopen=()=>{$('sDot').className='dot on';$('sText').textContent='Connecte';addLog('WS connecte')};
  ws.onclose=()=>{$('sDot').className='dot off';$('sText').textContent='Deconnecte';setTimeout(wsConnect,2000)};
  ws.onerror=()=>{ws.close()};
  ws.onmessage=e=>{try{handleWs(JSON.parse(e.data))}catch(x){}};
}
function wsSend(o){if(ws&&ws.readyState===1)ws.send(JSON.stringify(o))}

function handleWs(d){
  if(d.t==='status'){
    $('monState').textContent=STATES[d.state]||'?';
    $('monState').style.color=d.playing?'#e94560':'#4ecca3';
    if(d.heap){$('monHeap').textContent=(d.heap/1024|0)+'KB';$('heapBar').textContent=(d.heap/1024|0)+'KB'}
    updateCC(1,d.cc1);updateCC(2,d.cc2);updateCC(7,d.cc7);updateCC(11,d.cc11);
    if(d.pp!==undefined)$('progressFill').style.width=d.pp+'%';
    if(d.ppos!==undefined)$('progressText').textContent=fmt(d.ppos)+' / '+fmt(playerDuration);
    if(d.ps!==undefined){$('btnPlay').disabled=(d.ps===1);$('btnPause').disabled=(d.ps!==1);$('btnStop').disabled=(d.ps===0&&!fileLoaded)}
  }else if(d.t==='midi_loaded'){
    fileLoaded=true;playerDuration=d.duration||0;
    $('fName').textContent=d.file;$('fEvents').textContent=d.events;$('fDuration').textContent=fmt(d.duration);
    $('btnPlay').disabled=false;$('btnStop').disabled=false;addLog('MIDI: '+d.file)
  }else if(d.t==='midi_error'){addLog('ERR: '+d.msg)}
  else if(d.t==='audio'){
    const rms=Math.min(100,Math.round((d.rms||0)*500));
    $('vuFill').style.width=rms+'%';$('vuVal').textContent=rms+'%';
    $('vuFill').style.background=rms>60?'#e94560':rms>30?'#e9a645':'#4ecca3';
    if(d.midi>0){$('pitchNote').textContent=mn(d.midi);$('pitchHz').textContent=Math.round(d.hz)+' Hz';
      const c=d.cents||0;$('pitchCents').textContent=(c>=0?'+':'')+c.toFixed(0)+' ct';
      $('pitchCents').className='pitch-cents '+(Math.abs(c)<15?'ok':c>0?'sharp':'flat')}
    else{$('pitchNote').textContent='-';$('pitchHz').textContent='- Hz';$('pitchCents').textContent='-'}
  }else if(d.t==='acal_prog'){
    $('acalProgress').style.display='block';$('acalStep').textContent='Note '+(d.idx+1)+'/'+d.total+' '+d.note;
    $('acalFill').style.width=(((d.idx||0)/(d.total||1))*100)+'%';
    const sn={0:'Attente',1:'Prep',2:'Stab',3:'Sweep...',4:'Analyse',5:'OK'};
    $('acalState').textContent=sn[d.st]||'...'
  }else if(d.t==='acal_done'){
    autoCalRunning=false;$('btnAcalStart').style.display='';$('btnAcalStop').style.display='none';
    $('acalFill').style.width='100%';$('acalState').textContent='Termine !';addLog('Auto-cal OK');
    if(d.results){let h='';d.results.forEach(r=>{h+='<div style="display:flex;justify-content:space-between;font-size:.8em;padding:2px 0">'+
      '<span>'+r.name+'</span><span style="color:'+(r.ok?'#4ecca3':'#e94560')+'">'+(r.ok?r.min+'%-'+r.max+'%':'Echec')+'</span></div>'});
      $('acalResults').innerHTML=h;$('acalResults').style.display='block'}
    setTimeout(loadConfig,1000)
  }
}
function updateCC(n,v){if(v===undefined)return;const p=(v/127*100).toFixed(0);
  const b=$('ccBar'+n),t=$('ccV'+n);if(b)b.style.width=p+'%';if(t)t.textContent=v}

// --- Load config ---
function loadConfig(){
  fetch('/api/config').then(r=>r.json()).then(d=>{
    CFG=d;micDetected=d.mic||false;
    $('devName').textContent=d.device||'ServoFlute';
    buildKeyboard();buildFlute(CFG,'fluteSvg',false);
    if(micDetected){$('micSection').style.display='';wsSend({t:'mic_mon',on:1})}
    else $('micSection').style.display='none';
  }).catch(e=>addLog('Erreur config: '+e))
}

// --- KEYBOARD ---
function buildKeyboard(){
  const c=$('pianoKeys');c.innerHTML='';if(!CFG||!CFG.notes||!CFG.notes.length){c.innerHTML='<div style="color:#888;padding:16px;text-align:center">Aucune note</div>';return}
  CFG.notes.forEach(n=>{
    const name=mn(n.midi);const key=document.createElement('div');
    key.className='key'+(isBlack(n.midi)?' black':'');key.dataset.midi=n.midi;
    let dots='<span class="kf-row">';for(let f=0;f<CFG.num_fingers;f++)dots+='<span class="kf '+(n.fp[f]?'o':'c')+'"></span>';dots+='</span>';
    key.innerHTML='<span class="note-name">'+name+'</span><span class="note-midi">'+n.midi+'</span>'+dots;
    key.addEventListener('touchstart',e=>{e.preventDefault();noteOn(n.midi);key.classList.add('pressed')},{passive:false});
    key.addEventListener('touchend',e=>{e.preventDefault();noteOff(n.midi);key.classList.remove('pressed')},{passive:false});
    key.addEventListener('mousedown',e=>{e.preventDefault();noteOn(n.midi);key.classList.add('pressed')});
    key.addEventListener('mouseup',()=>{noteOff(n.midi);key.classList.remove('pressed')});
    key.addEventListener('mouseleave',()=>{if(key.classList.contains('pressed')){noteOff(n.midi);key.classList.remove('pressed')}});
    c.appendChild(key)
  });
  buildKeyMap()
}

function noteOn(midi){curNote=midi;updateFluteForNote(midi);wsSend({t:'non',n:midi,v:velocity})}
function noteOff(midi){wsSend({t:'nof',n:midi});if(curNote===midi)curNote=null}
function setVelocity(v){velocity=parseInt(v);$('velVal').textContent=v;wsSend({t:'velocity',v:velocity})}

// Keyboard shortcuts
const KC='azertyuiopqsdfghjklmwxcvbn'.split('');let keyMap={},keysDown=new Set();
function buildKeyMap(){keyMap={};if(!CFG)return;CFG.notes.forEach((n,i)=>{if(i<KC.length)keyMap[KC[i]]=n.midi})}
document.addEventListener('keydown',e=>{if(e.target.tagName==='INPUT'||e.target.tagName==='SELECT'||e.repeat)return;
  const n=keyMap[e.key.toLowerCase()];if(n&&!keysDown.has(e.key)){keysDown.add(e.key);noteOn(n);
    const el=document.querySelector('.key[data-midi="'+n+'"]');if(el)el.classList.add('pressed')}});
document.addEventListener('keyup',e=>{const n=keyMap[e.key.toLowerCase()];if(n){keysDown.delete(e.key);noteOff(n);
    const el=document.querySelector('.key[data-midi="'+n+'"]');if(el)el.classList.remove('pressed')}});

// --- SVG FLUTE ---
function buildFlute(cfg,svgId,showNums){
  const svg=$(svgId);if(!svg||!cfg)return;
  const nf=cfg.num_fingers||6,fingers=cfg.fingers||[];
  const sp=50,sx=80,gap=30,r=14;
  const topHoles=[],botHoles=[];
  for(let i=0;i<nf;i++){(fingers[i]&&fingers[i].th?botHoles:topHoles).push(i)}
  // Calculate positions
  const lc=Math.ceil(topHoles.length/2),rc=topHoles.length-lc;
  const posTop=[];
  for(let i=0;i<topHoles.length;i++){
    const x=sx+(i<lc?i:(lc+1))*sp+(i>=lc?gap:0);posTop.push(x)
  }
  const posBot=[];
  for(let i=0;i<botHoles.length;i++){posBot.push(sx+i*sp)}
  const allX=[...posTop,...posBot,sx+200];
  const tw=Math.max(...allX)+60;
  const h_top=35,h_bot=65,cy=50;
  svg.setAttribute('viewBox','0 0 '+tw+' 100');
  let h='<defs><linearGradient id="wg_'+svgId+'" x1="0" y1="0" x2="0" y2="1">';
  h+='<stop offset="0%" stop-color="#C4A035"/><stop offset="45%" stop-color="#9B7A1C"/>';
  h+='<stop offset="100%" stop-color="#6B4F10"/></linearGradient></defs>';
  h+='<rect x="15" y="28" width="'+(tw-30)+'" height="44" rx="22" fill="url(#wg_'+svgId+')" stroke="#5C4A0A" stroke-width="2"/>';
  h+='<ellipse cx="24" cy="'+cy+'" rx="13" ry="24" fill="#B08C20" stroke="#5C4A0A" stroke-width="2"/>';
  // Top holes
  topHoles.forEach((fi,i)=>{
    h+='<circle id="fh_'+svgId+'_'+fi+'" cx="'+posTop[i]+'" cy="'+h_top+'" r="'+r+'" class="flute-hole closed"/>';
    if(showNums)h+='<text x="'+posTop[i]+'" y="'+(h_top+4)+'" text-anchor="middle" class="flute-num">'+(fi+1)+'</text>'
  });
  // Bottom holes (thumb)
  botHoles.forEach((fi,i)=>{
    h+='<circle id="fh_'+svgId+'_'+fi+'" cx="'+posBot[i]+'" cy="'+h_bot+'" r="'+(r-2)+'" class="flute-hole closed thumb"/>';
    if(showNums)h+='<text x="'+posBot[i]+'" y="'+(h_bot+4)+'" text-anchor="middle" class="flute-num">'+(fi+1)+'</text>'
  });
  // Hand separator
  if(topHoles.length>1&&lc>0&&rc>0){
    const sepX=(posTop[lc-1]+posTop[lc])/2;
    h+='<line x1="'+sepX+'" y1="22" x2="'+sepX+'" y2="50" stroke="#5C4A0A" stroke-width="1" stroke-dasharray="3,3" opacity="0.4"/>';
    h+='<text x="'+((posTop[0]+posTop[lc-1])/2)+'" y="18" text-anchor="middle" class="flute-lbl">G</text>';
    h+='<text x="'+((posTop[lc]+posTop[topHoles.length-1])/2)+'" y="18" text-anchor="middle" class="flute-lbl">D</text>'
  }
  if(botHoles.length>0)h+='<text x="'+(posBot[0])+'" y="88" text-anchor="middle" class="flute-lbl">Pouce</text>';
  svg.innerHTML=h
}

function updateFluteForNote(midi){
  if(!CFG)return;const nd=CFG.notes.find(n=>n.midi===midi);
  for(let i=0;i<CFG.num_fingers;i++){const el=$('fh_fluteSvg_'+i);
    if(el)el.setAttribute('class','flute-hole '+(nd&&nd.fp[i]?'open':'closed')+(CFG.fingers[i]&&CFG.fingers[i].th?' thumb':''))}
  $('fluteNote').textContent=nd?mn(nd.midi):'-';$('fluteInfo').textContent=nd?'MIDI '+nd.midi:''
}

// --- MIDI UPLOAD ---
const dz=$('dropZone');
dz.addEventListener('dragover',e=>{e.preventDefault();dz.classList.add('hover')});
dz.addEventListener('dragleave',()=>dz.classList.remove('hover'));
dz.addEventListener('drop',e=>{e.preventDefault();dz.classList.remove('hover');
  if(e.dataTransfer.files.length)uploadMidiFile(e.dataTransfer.files[0])});
function uploadMidi(input){if(input.files.length)uploadMidiFile(input.files[0])}
function uploadMidiFile(file){
  const fd=new FormData();fd.append('file',file);
  fetch('/api/midi',{method:'POST',body:fd}).then(r=>r.json()).then(d=>{
    if(d.ok){addLog('Upload OK: '+d.events+' evt')}else{addLog('ERR: '+(d.msg||'echec'))}
  }).catch(e=>addLog('Upload erreur: '+e))
}

// --- CALIBRATION ---
function buildCalibUI(){if(!CFG)return;buildFlute(CFG,'calFluteSvg',true);buildFingerCards();goStep(calibStep)}

function goStep(s){
  calibStep=s;
  ['step1','step2','step3'].forEach((id,i)=>{$(id).style.display=(i+1===s)?'':'none'});
  document.querySelectorAll('.step-dot').forEach((d,i)=>{d.className='step-dot'+(i+1===s?' active':i+1<s?' done':'')});
  if(s===2)buildFingeringRows();
  if(s===3)buildAirflowRows()
}

function changeFingers(delta){
  if(!CFG)return;let nf=CFG.num_fingers+delta;
  if(nf<1)nf=1;if(nf>15)nf=15;CFG.num_fingers=nf;
  // Add defaults for new fingers
  while(CFG.fingers.length<nf)CFG.fingers.push({ch:CFG.fingers.length,a:90,d:1,th:0});
  $('numFingersDisp').textContent=nf;buildFingerCards();buildFlute(CFG,'calFluteSvg',true)
}

function buildFingerCards(){
  const c=$('fingerCards');c.innerHTML='';if(!CFG)return;
  $('numFingersDisp').textContent=CFG.num_fingers;
  $('angleOpen').value=CFG.angle_open||30;$('aoVal').textContent=(CFG.angle_open||30)+'deg';
  // PCA dropdown
  const sel=$('airPca');sel.innerHTML='';
  for(let i=0;i<16;i++){const o=document.createElement('option');o.value=i;o.textContent='PCA '+i;sel.appendChild(o)}
  sel.value=CFG.air_pca||10;
  // Finger cards
  for(let i=0;i<CFG.num_fingers;i++){
    const f=CFG.fingers[i]||{ch:i,a:90,d:1,th:0};
    const d=document.createElement('div');d.className='cal-card';
    d.innerHTML='<h4>Doigt '+(i+1)+' <span style="color:#888;font-size:.85em">(PCA '+f.ch+')</span></h4>'+
      '<div class="cfg-row"><label>PCA</label><select id="fch'+i+'" style="max-width:80px" onchange="CFG.fingers['+i+'].ch=parseInt(this.value)">'+
        Array.from({length:16},(_,j)=>'<option value="'+j+'"'+(j===f.ch?' selected':'')+'>'+j+'</option>').join('')+'</select></div>'+
      '<div class="cfg-row"><label>Angle ferme</label><input type="range" min="0" max="180" value="'+f.a+'" id="fa'+i+
        '" oninput="CFG.fingers['+i+'].a=parseInt(this.value);$(\'fav'+i+'\').textContent=this.value+\'&deg;\';testFinger('+i+',parseInt(this.value))">'+
        '<span id="fav'+i+'" style="min-width:36px">'+f.a+'&deg;</span></div>'+
      '<div class="cfg-row"><label>Direction</label><select id="fd'+i+'" style="max-width:80px" onchange="CFG.fingers['+i+'].d=parseInt(this.value)">'+
        '<option value="1"'+(f.d===1?' selected':'')+'>+1</option><option value="-1"'+(f.d===-1?' selected':'')+'>-1</option></select></div>'+
      '<div class="cfg-row"><label>Trou arriere</label><input type="checkbox" id="fth'+i+'"'+(f.th?' checked':'')+
        ' style="width:auto;flex:0" onchange="CFG.fingers['+i+'].th=this.checked?1:0;buildFlute(CFG,\'calFluteSvg\',true)"></div>'+
      '<div class="btn-row"><button class="btn btn-s" onclick="testFinger('+i+',CFG.fingers['+i+'].a)">Fermer</button>'+
        '<button class="btn btn-s" onclick="testFinger('+i+',CFG.fingers['+i+'].a+(CFG.angle_open||30)*CFG.fingers['+i+'].d)">Ouvrir</button></div>';
    c.appendChild(d)
  }
}

function testFinger(i,a){wsSend({t:'test_finger',i:i,a:parseInt(a)});
  // Update flute SVG
  const el=$('fh_calFluteSvg_'+i);if(el){const closed=CFG.fingers[i].a;const open=Math.abs(a-closed)>(CFG.angle_open||30)/2;
    el.setAttribute('class','flute-hole '+(open?'open':'closed')+(CFG.fingers[i].th?' thumb':''))}}

function saveStep1(){
  if(!CFG)return;
  CFG.angle_open=parseInt($('angleOpen').value);CFG.air_pca=parseInt($('airPca').value);
  // Collect finger data
  for(let i=0;i<CFG.num_fingers;i++){
    CFG.fingers[i].ch=parseInt($('fch'+i).value);
    CFG.fingers[i].a=parseInt($('fa'+i).value);
    CFG.fingers[i].d=parseInt($('fd'+i).value);
    CFG.fingers[i].th=$('fth'+i).checked?1:0
  }
  const body={num_fingers:CFG.num_fingers,air_pca:CFG.air_pca,angle_open:CFG.angle_open,fingers:CFG.fingers.slice(0,CFG.num_fingers)};
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})
    .then(r=>r.json()).then(d=>{if(d.ok){addLog('Doigts sauves');goStep(2)}else addLog('Erreur sauvegarde')})
    .catch(e=>addLog('Erreur: '+e))
}

// --- STEP 2: FINGERINGS ---
function buildFingeringRows(){
  const c=$('fingeringRows');c.innerHTML='';if(!CFG)return;
  CFG.notes.forEach((n,ni)=>{
    const d=document.createElement('div');d.className='fg-row';
    let dots='';for(let f=0;f<CFG.num_fingers;f++){
      const isThumb=CFG.fingers[f]&&CFG.fingers[f].th;
      dots+='<div class="fg-dot '+(n.fp[f]?'open':'closed')+(isThumb?' thumb':'')+'" data-ni="'+ni+'" data-fi="'+f+'" onclick="toggleFP('+ni+','+f+',this)"></div>'
    }
    d.innerHTML='<input type="number" class="fg-midi" style="width:48px" value="'+n.midi+'" min="0" max="127" onchange="CFG.notes['+ni+'].midi=parseInt(this.value)">'+
      '<span class="fg-note">'+mn(n.midi)+'</span>'+
      '<div class="fg-dots">'+dots+'</div>'+
      '<button class="btn btn-s" style="padding:4px 8px;font-size:.75em" onclick="wsSend({t:\'test_note\',n:'+n.midi+'})">Test</button>';
    c.appendChild(d)
  })
}

function toggleFP(ni,fi,el){
  CFG.notes[ni].fp[fi]=CFG.notes[ni].fp[fi]?0:1;
  el.className='fg-dot '+(CFG.notes[ni].fp[fi]?'open':'closed')+(CFG.fingers[fi]&&CFG.fingers[fi].th?' thumb':'')
}

function addNote(){
  if(!CFG)return;const last=CFG.notes.length?CFG.notes[CFG.notes.length-1]:null;
  const midi=last?last.midi+2:84;const fp=new Array(CFG.num_fingers).fill(0);
  CFG.notes.push({midi:midi,fp:fp,amn:20,amx:75});CFG.num_notes=CFG.notes.length;buildFingeringRows()
}
function removeLastNote(){if(!CFG||!CFG.notes.length)return;CFG.notes.pop();CFG.num_notes=CFG.notes.length;buildFingeringRows()}

function applyPreset(val){
  if(!val||!CFG)return;
  // Only Irish C preset built-in for now
  if(val==='irish_c'){
    CFG.notes=[
      {midi:82,fp:[0,1,1,1,1,1],amn:10,amx:60},{midi:83,fp:[1,1,1,1,1,1],amn:0,amx:50},
      {midi:84,fp:[0,0,0,0,0,0],amn:20,amx:75},{midi:86,fp:[0,0,0,0,0,1],amn:15,amx:70},
      {midi:88,fp:[0,0,0,0,1,1],amn:10,amx:65},{midi:89,fp:[0,0,0,1,1,1],amn:10,amx:60},
      {midi:91,fp:[0,0,1,1,1,1],amn:5,amx:55},{midi:93,fp:[0,1,1,1,1,1],amn:5,amx:50},
      {midi:95,fp:[1,1,1,1,1,1],amn:0,amx:45},
      {midi:96,fp:[0,0,0,0,0,0],amn:50,amx:100},{midi:98,fp:[0,0,0,0,0,1],amn:45,amx:95},
      {midi:100,fp:[0,0,0,0,1,1],amn:40,amx:90},{midi:101,fp:[0,0,0,1,1,1],amn:35,amx:85},
      {midi:103,fp:[0,0,1,1,1,1],amn:30,amx:80}
    ];
    // Pad fp arrays to num_fingers
    CFG.notes.forEach(n=>{while(n.fp.length<CFG.num_fingers)n.fp.push(0)});
    CFG.num_notes=CFG.notes.length;buildFingeringRows()
  }
}

function saveStep2(){
  if(!CFG)return;
  const body={notes:CFG.notes.map(n=>({midi:n.midi,fp:n.fp.slice(0,CFG.num_fingers),amn:n.amn,amx:n.amx}))};
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})
    .then(r=>r.json()).then(d=>{if(d.ok){addLog('Doigtes sauves');goStep(3);buildKeyboard()}else addLog('Erreur')})
    .catch(e=>addLog('Erreur: '+e))
}

// --- STEP 3: AIRFLOW ---
function buildAirflowRows(){
  const c=$('airflowRows');c.innerHTML='';if(!CFG)return;
  CFG.notes.forEach((n,ni)=>{
    let dots='';for(let f=0;f<CFG.num_fingers;f++)dots+='<span class="kf '+(n.fp[f]?'o':'c')+'"></span>';
    const d=document.createElement('div');d.className='air-card';
    d.innerHTML='<span class="air-note">'+mn(n.midi)+'</span>'+
      '<span class="kf-row" style="gap:2px">'+dots+'</span>'+
      '<div class="air-sliders">'+
        '<div class="air-vals"><span>Min: <b id="amn'+ni+'">'+n.amn+'</b>%</span><span>Max: <b id="amx'+ni+'">'+n.amx+'</b>%</span></div>'+
        '<input type="range" min="0" max="100" value="'+n.amn+'" oninput="CFG.notes['+ni+'].amn=parseInt(this.value);$(\'amn'+ni+'\').textContent=this.value">'+
        '<input type="range" min="0" max="100" value="'+n.amx+'" oninput="CFG.notes['+ni+'].amx=parseInt(this.value);$(\'amx'+ni+'\').textContent=this.value">'+
      '</div>'+
      '<button class="btn btn-s" style="padding:4px 8px;font-size:.75em" onclick="testCalNote('+n.midi+')">Test</button>';
    c.appendChild(d)
  })
}

function testCalNote(midi){wsSend({t:'test_note',n:midi});wsSend({t:'test_sol',o:1});
  setTimeout(()=>wsSend({t:'test_sol',o:0}),2000)}

function startAutoCal(){autoCalRunning=true;$('btnAcalStart').style.display='none';$('btnAcalStop').style.display='';
  $('acalProgress').style.display='block';$('acalResults').style.display='none';wsSend({t:'auto_cal',mode:'air'})}
function stopAutoCal(){autoCalRunning=false;$('btnAcalStart').style.display='';$('btnAcalStop').style.display='none';
  wsSend({t:'auto_cal',mode:'stop'})}

function saveStep3(){
  if(!CFG)return;
  const body={notes_air:CFG.notes.map(n=>({amn:n.amn,amx:n.amx}))};
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})
    .then(r=>r.json()).then(d=>{if(d.ok){addLog('Souffle sauve');buildKeyboard();alert('Calibration terminee !')}else addLog('Erreur')})
    .catch(e=>addLog('Erreur: '+e))
}

// --- SETTINGS ---
function fillSettings(){
  if(!CFG)return;
  $('cfgDevice').value=CFG.device||'';
  const sel=$('cfgMidiCh');sel.innerHTML='<option value="0">Omni (tous)</option>';
  for(let i=1;i<=16;i++){const o=document.createElement('option');o.value=i;o.textContent='Canal '+i;sel.appendChild(o)}
  sel.value=CFG.midi_ch||0;
  $('cfgDelay').value=CFG.servo_delay;$('cfgValveInt').value=CFG.valve_interval;$('cfgMinNote').value=CFG.min_note_dur;
  $('cfgAirOff').value=CFG.air_off;$('cfgAirMin').value=CFG.air_min;$('cfgAirMax').value=CFG.air_max;
  $('cfgVibF').value=CFG.vib_freq;$('cfgVibA').value=CFG.vib_amp;
  $('cfgCC2On').checked=CFG.cc2_on;$('cfgCC2Thr').value=CFG.cc2_thr;$('cfgCC2Curve').value=CFG.cc2_curve;$('cfgCC2To').value=CFG.cc2_timeout;
  $('cfgSolAct').value=CFG.sol_act;$('cfgSolHold').value=CFG.sol_hold;$('cfgSolTime').value=CFG.sol_time;
  $('cfgUnpower').value=CFG.time_unpower;
  $('wifiSsid').value=CFG.wifi_ssid||'';
  // WiFi status
  fetch('/api/wifi/status').then(r=>r.json()).then(d=>{
    $('wifiState').textContent=d.ap?'AP: '+d.ip:'STA: '+(d.ip||'deconnecte')+(d.rssi?' ('+d.rssi+' dBm)':'')
  }).catch(()=>{})
}

function saveSettings(){
  const body={device:$('cfgDevice').value,midi_ch:parseInt($('cfgMidiCh').value),
    servo_delay:parseInt($('cfgDelay').value),valve_interval:parseInt($('cfgValveInt').value),
    min_note_dur:parseInt($('cfgMinNote').value),
    air_off:parseInt($('cfgAirOff').value),air_min:parseInt($('cfgAirMin').value),air_max:parseInt($('cfgAirMax').value),
    vib_freq:parseFloat($('cfgVibF').value),vib_amp:parseFloat($('cfgVibA').value),
    cc2_on:$('cfgCC2On').checked,cc2_thr:parseInt($('cfgCC2Thr').value),
    cc2_curve:parseFloat($('cfgCC2Curve').value),cc2_timeout:parseInt($('cfgCC2To').value),
    sol_act:parseInt($('cfgSolAct').value),sol_hold:parseInt($('cfgSolHold').value),sol_time:parseInt($('cfgSolTime').value),
    time_unpower:parseInt($('cfgUnpower').value)};
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})
    .then(r=>r.json()).then(d=>{$('settingsMsg').textContent=d.ok?'Sauvegarde OK':'Erreur';
      $('settingsMsg').style.color=d.ok?'#4ecca3':'#e94560';if(d.ok)loadConfig()})
    .catch(e=>{$('settingsMsg').textContent='Erreur: '+e;$('settingsMsg').style.color='#e94560'})
}

function resetConfig(){if(!confirm('Remettre tous les parametres par defaut ?'))return;
  fetch('/api/config/reset',{method:'POST'}).then(r=>r.json()).then(d=>{if(d.ok){addLog('Reset OK');loadConfig();fillSettings()}})
    .catch(e=>addLog('Erreur: '+e))}

// --- WIFI ---
function startWifiScan(){$('scanStatus').textContent='Scan...';
  fetch('/api/wifi/scan').then(()=>{setTimeout(checkScan,3000)}).catch(e=>{$('scanStatus').textContent='Erreur'})}
function checkScan(){fetch('/api/wifi/results').then(r=>r.json()).then(d=>{
    if(!d.done){setTimeout(checkScan,2000);return}
    $('scanStatus').textContent='';const c=$('wifiList');c.innerHTML='';
    if(d.networks)d.networks.forEach(n=>{
      const el=document.createElement('div');el.className='wifi-item';
      el.innerHTML='<span>'+n.ssid+'</span><span style="color:#888">'+n.rssi+' dBm</span>';
      el.onclick=()=>{$('wifiSsid').value=n.ssid};c.appendChild(el)})
  }).catch(()=>{$('scanStatus').textContent='Erreur'})}

function connectWifi(){const ssid=$('wifiSsid').value,pass=$('wifiPass').value;
  if(!ssid){$('wifiMsg').textContent='SSID requis';return}
  $('wifiMsg').textContent='Connexion...';
  fetch('/api/wifi/connect',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({ssid:ssid,pass:pass})})
    .then(r=>r.json()).then(d=>{$('wifiMsg').textContent=d.msg||'OK'})
    .catch(e=>{$('wifiMsg').textContent='Erreur: '+e})}

// --- INIT ---
window.addEventListener('load',()=>{wsConnect();loadConfig()});
</script>
</body>
</html>
)rawliteral";

#endif
