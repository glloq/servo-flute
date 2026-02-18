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
/* Flute graphic */
.flute-section{background:#16213e;border-radius:10px;padding:14px;margin:8px 0}
.flute-note-display{text-align:center;margin-bottom:10px}
.flute-note-display .fn{font-size:2em;font-weight:bold;color:#e94560;display:block}
.flute-note-display .fs{font-size:0.75em;color:#888;display:block;margin-top:2px}
.flute-svg{width:100%;max-width:480px;display:block;margin:0 auto}
.flute-hole{stroke:#5C4A0A;stroke-width:2;transition:fill .12s,filter .12s}
.flute-hole.closed{fill:#2a1a0a}.flute-hole.open{fill:#4ecca3;filter:drop-shadow(0 0 6px rgba(78,204,163,0.7))}
.flute-lbl{fill:#777;font-size:11px}
.flute-hands{display:flex;justify-content:space-around;font-size:0.7em;color:#666;max-width:480px;margin:2px auto 0}
/* Air control */
.air-section{background:#16213e;border-radius:10px;padding:14px;margin:8px 0}
.air-header{display:flex;justify-content:space-between;align-items:center;margin-bottom:6px;font-size:0.85em}
.air-pct{font-weight:bold;color:#4ecca3;font-size:1.1em}
.air-track{position:relative;height:34px;margin:4px 0}
.air-bg{position:absolute;top:11px;left:0;right:0;height:12px;background:#0a0a1a;border-radius:6px}
.air-fill{position:absolute;top:11px;height:12px;background:rgba(78,204,163,0.2);
border-left:2px solid #4ecca3;border-right:2px solid #4ecca3;border-radius:6px;transition:left .2s,width .2s}
.air-track input[type=range]{position:absolute;top:0;left:0;width:100%;height:34px;
-webkit-appearance:none;appearance:none;background:transparent;z-index:2;cursor:pointer;margin:0;padding:0}
.air-track input[type=range]::-webkit-slider-runnable-track{height:12px;background:transparent;margin-top:5px}
.air-track input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:22px;height:22px;
border-radius:50%;background:#e94560;border:2px solid #fff;cursor:pointer;margin-top:-5px}
.air-track input[type=range]::-moz-range-track{height:12px;background:transparent}
.air-track input[type=range]::-moz-range-thumb{width:18px;height:18px;border-radius:50%;
background:#e94560;border:2px solid #fff;cursor:pointer}
.air-labels{display:flex;justify-content:space-between;font-size:0.7em;color:#888}
.air-off{opacity:0.35;pointer-events:none}
/* Key finger dots */
.kf-row{display:flex;gap:2px;justify-content:center;margin-top:4px}
.kf{width:5px;height:5px;border-radius:50%;border:1px solid #555}
.kf.o{background:#4ecca3;border-color:#4ecca3}.kf.c{background:#2a1a0a;border-color:#555}
.key .air-info{font-size:0.6em;color:#666;margin-top:2px;display:block}
/* Calibration flute mini */
.cal-flute-wrap{background:#0a0a1a;border-radius:8px;padding:10px;margin:8px 0;text-align:center}
.cal-flute-wrap svg{width:100%;max-width:420px}
.cal-flute-info{font-size:0.75em;color:#888;margin-top:4px}
/* Note calibration panel */
.note-cal{background:#0f3460;border:1px solid #0f3460;border-radius:10px;padding:14px;margin:10px 0;
display:none;transition:all .2s}
.note-cal.active{display:block;border-color:#e94560}
.note-cal-header{display:flex;justify-content:space-between;align-items:center;margin-bottom:10px}
.note-cal-header .ncn{font-size:1.4em;font-weight:bold;color:#e94560}
.note-cal-header .ncm{font-size:0.8em;color:#888;margin-left:8px}
.note-cal-actions{display:flex;gap:6px;margin-top:10px;flex-wrap:wrap}
/* Visual fingering table */
.fing-tbl{width:100%;border-collapse:collapse;font-size:0.82em}
.fing-tbl th{text-align:center;color:#e94560;padding:4px 3px;border-bottom:1px solid #0f3460;font-size:0.85em}
.fing-tbl td{padding:5px 3px;border-bottom:1px solid #0a0a1a;text-align:center;vertical-align:middle}
.fing-tbl tr:hover{background:rgba(78,204,163,0.05)}
.fing-dot{width:16px;height:16px;border-radius:50%;display:inline-block;cursor:pointer;
border:2px solid #555;transition:all .15s;vertical-align:middle}
.fing-dot.open{background:#4ecca3;border-color:#4ecca3;box-shadow:0 0 6px rgba(78,204,163,0.5)}
.fing-dot.closed{background:#2a1a0a;border-color:#555}
.fing-dot:hover{transform:scale(1.2);border-color:#e94560}
.fing-dots-cell{display:flex;gap:3px;justify-content:center;align-items:center}
.fing-sep{width:1px;height:14px;background:#333;margin:0 2px}
/* Sweep tool */
.sweep-panel{background:#0a0a1a;border-radius:8px;padding:12px;margin:8px 0;display:none}
.sweep-panel.active{display:block}
.sweep-bar{height:24px;background:#16213e;border-radius:12px;position:relative;margin:8px 0;overflow:hidden}
.sweep-cursor{position:absolute;top:0;width:4px;height:100%;background:#e94560;transition:left 0.1s linear}
.sweep-range{position:absolute;top:0;height:100%;background:rgba(78,204,163,0.3);
border-left:2px solid #4ecca3;border-right:2px solid #4ecca3}
.sweep-btns{display:flex;gap:8px;flex-wrap:wrap}
.sweep-btns button{flex:1;min-width:90px}
/* Wizard */
.wiz-overlay{position:fixed;top:0;left:0;right:0;bottom:0;background:rgba(0,0,0,0.85);
z-index:100;display:none;justify-content:center;align-items:center;padding:16px}
.wiz-overlay.active{display:flex}
.wiz-box{background:#16213e;border-radius:12px;padding:20px;max-width:420px;width:100%;
max-height:90vh;overflow-y:auto;border:2px solid #0f3460}
.wiz-title{font-size:1.1em;color:#e94560;margin-bottom:6px}
.wiz-step-info{font-size:0.8em;color:#888;margin-bottom:14px}
.wiz-progress{display:flex;gap:4px;margin:10px 0;justify-content:center;flex-wrap:wrap}
.wiz-progress .dot{width:8px;height:8px;border-radius:50%;background:#333}
.wiz-progress .dot.done{background:#4ecca3}.wiz-progress .dot.cur{background:#e94560}
.wiz-finger-row{display:flex;align-items:center;gap:8px;margin:4px 0;padding:6px;
background:#0a0a1a;border-radius:6px}
.wiz-finger-row span:first-child{color:#888;min-width:60px;font-size:0.85em}
.wiz-finger-row .wiz-angle{color:#4ecca3;font-weight:bold;min-width:35px}
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
  <!-- Representation graphique de la flute -->
  <div class="flute-section">
    <div class="flute-note-display">
      <span class="fn" id="fluteNote">-</span>
      <span class="fs" id="fluteInfo">Selectionnez une note</span>
    </div>
    <svg class="flute-svg" id="fluteSvg" xmlns="http://www.w3.org/2000/svg">
    </svg>
  </div>

  <!-- Controle du souffle (debit d'air) -->
  <div class="air-section air-off" id="airSection">
    <div class="air-header">
      <span>Souffle (debit d'air)</span>
      <span class="air-pct" id="airPct">-</span>
    </div>
    <div class="air-track">
      <div class="air-bg"></div>
      <div class="air-fill" id="airFill" style="left:0%;width:100%"></div>
      <input type="range" id="airSlider" min="0" max="127" value="64"
        oninput="onAirChange(this.value)">
    </div>
    <div class="air-labels">
      <span id="airMinL">Min: -</span>
      <span id="airMaxL">Max: -</span>
    </div>
  </div>

  <!-- Clavier des notes jouables -->
  <div class="keyboard" id="pianoKeys">
    <div style="text-align:center;color:#888;padding:20px">Chargement des notes...</div>
  </div>

  <div class="slider-group">
    <label>Velocity</label>
    <input type="range" id="velSlider" min="1" max="127" value="100"
      oninput="setVelocity(this.value)">
    <span class="val" id="velVal">100</span>
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
  <!-- Mini flute visual feedback -->
  <div class="cal-flute-wrap">
    <svg class="flute-svg" id="calFluteSvg" xmlns="http://www.w3.org/2000/svg"></svg>
    <div class="cal-flute-info" id="calFluteInfo">Bougez un slider pour voir l'effet</div>
  </div>

  <div class="section">
    <h3>Servos doigts
      <button class="btn-action" style="float:right;font-size:0.8em" onclick="startWizard()">Assistant</button>
    </h3>
    <div id="calFingers">Chargement...</div>
  </div>

  <div class="section">
    <h3>Airflow servo</h3>
    <div class="cal-servo">
      <div class="cal-label">Servo airflow</div>
      <div class="cal-val" id="calAirVal">20</div>
      <input type="range" class="cal-range" id="calAirSlider" min="0" max="180" value="20"
        oninput="testAirflow(this.value)">
    </div>
  </div>

  <div class="section">
    <h3>Solenoide</h3>
    <div style="display:flex;gap:8px">
      <button class="btn-action" style="flex:1" onclick="testSolenoid(1)">OUVRIR</button>
      <button class="btn-action" style="flex:1" onclick="testSolenoid(0)">FERMER</button>
    </div>
  </div>

  <div class="section">
    <h3>Calibration par note</h3>
    <div class="cfg-row">
      <label>Note</label>
      <select id="calNoteSelect" onchange="selectCalNote()"></select>
      <button class="btn-action" onclick="testNote()" style="margin-left:8px">Tester position</button>
    </div>
    <!-- Contextual note calibration panel -->
    <div class="note-cal" id="noteCalPanel">
      <div class="note-cal-header">
        <div><span class="ncn" id="ncNoteName">-</span><span class="ncm" id="ncNoteInfo"></span></div>
      </div>
      <div class="cal-servo">
        <div class="cal-label">Air min % <span style="color:#4ecca3;float:right" id="ncAirMinVal">-</span></div>
        <input type="range" class="cal-range" id="ncAirMin" min="0" max="100" value="0"
          oninput="onNcAirChange()">
      </div>
      <div class="cal-servo">
        <div class="cal-label">Air max % <span style="color:#4ecca3;float:right" id="ncAirMaxVal">-</span></div>
        <input type="range" class="cal-range" id="ncAirMax" min="0" max="100" value="100"
          oninput="onNcAirChange()">
      </div>
      <!-- Sweep tool -->
      <div class="sweep-panel" id="sweepPanel">
        <div style="font-size:0.85em;margin-bottom:6px;color:#a0a0a0">Sweep automatique du souffle</div>
        <div class="sweep-bar" id="sweepBar">
          <div class="sweep-range" id="sweepRange" style="display:none"></div>
          <div class="sweep-cursor" id="sweepCursor" style="left:0%"></div>
        </div>
        <div style="font-size:0.85em;color:#4ecca3;text-align:center;font-weight:bold" id="sweepPct">0%</div>
        <div class="sweep-btns">
          <button class="btn-action" id="btnSweepMin" onclick="markSweepMin()" disabled>Debut du son</button>
          <button class="btn-action" id="btnSweepMax" onclick="markSweepMax()" disabled>Fin du son</button>
        </div>
      </div>
      <div class="note-cal-actions">
        <button class="btn-action" onclick="startSweep()">Sweep souffle</button>
        <button class="btn-action" onclick="testCalNote()">Tester</button>
        <button class="btn-save" style="padding:8px 16px;font-size:0.85em" onclick="saveNoteCalibration()">Sauver</button>
      </div>
    </div>
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
    <h3>Doigtes et airflow par note</h3>
    <div style="overflow-x:auto">
    <table class="fing-tbl" id="fingeringTable">
      <thead><tr><th>Note</th><th>MIDI</th><th>Doigtes</th><th>Air min</th><th>Air max</th><th></th></tr></thead>
      <tbody id="fingeringBody"></tbody>
    </table>
    </div>
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

<!-- Wizard overlay -->
<div class="wiz-overlay" id="wizOverlay">
  <div class="wiz-box">
    <div class="wiz-title" id="wizTitle">Assistant calibration</div>
    <div class="wiz-step-info" id="wizStepInfo"></div>
    <div class="wiz-progress" id="wizProgress"></div>
    <div id="wizBody"></div>
    <div class="wiz-nav">
      <button class="btn-secondary" id="wizPrev" onclick="wizPrev()" style="display:none">Retour</button>
      <button class="btn-save" id="wizNext" onclick="wizNext()">Valider</button>
      <button class="btn-action" onclick="wizClose()" style="min-width:auto;padding:8px 12px">Fermer</button>
    </div>
  </div>
</div>

<script>
const NOTE_NAMES=['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'];
const STATES=["IDLE","POSITIONING","PLAYING","STOPPING"];
let ws=null,velocity=100,wsConnected=false;
let cfgData=null,notesData=[],numFingers=6;
let curNote=null,curNoteData=null;
let calNoteIdx=-1,sweepTimer=null,sweepPct=0,sweepMin=-1,sweepMax=-1;
let wizStep=0,wizFingerData=[];

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
    let dots='';
    if(n.fingers){dots='<span class="kf-row">';n.fingers.forEach(f=>{dots+='<span class="kf '+(f?'o':'c')+'"></span>'});dots+='</span>'}
    key.innerHTML='<span class="note-name">'+name+'</span><span class="note-midi">MIDI '+n.midi+'</span>'+dots+'<span class="air-info">'+n.air_min+'-'+n.air_max+'%</span>';
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

function noteOn(midi){
  curNote=midi;curNoteData=notesData.find(n=>n.midi===midi)||null;
  updateFlute(curNoteData);updateAir(curNoteData);
  if(curNoteData){const mc=Math.round((curNoteData.air_min+curNoteData.air_max)/2*1.27);wsSend({t:'cc',c:2,v:mc})}
  wsSend({t:'non',n:midi,v:velocity});addLog("ON: "+midiToName(midi)+" ("+midi+")")
}
function noteOff(midi){wsSend({t:'nof',n:midi});if(curNote===midi)curNote=null}
function sendCC(num,val){wsSend({t:'cc',c:parseInt(num),v:parseInt(val)})}

function updateFlute(nd){
  for(let i=0;i<numFingers;i++){const h=$('fh'+i);if(!h)continue;
    h.setAttribute('class','flute-hole '+(nd&&nd.fingers&&nd.fingers[i]?'open':'closed'))}
  if(nd){$('fluteNote').textContent=midiToName(nd.midi);
    $('fluteInfo').textContent='MIDI '+nd.midi+' | Souffle: '+nd.air_min+'-'+nd.air_max+'%'}
  else{$('fluteNote').textContent='-';$('fluteInfo').textContent='Selectionnez une note'}
}
function buildFlute(){
  const svg=$('fluteSvg');if(!svg)return;
  const nf=numFingers,sp=50,sx=80,gap=30,cy=40,r=14;
  const lc=Math.ceil(nf/2),rc=nf-lc;
  const pos=[];
  for(let i=0;i<lc;i++)pos.push(sx+i*sp);
  const rx=sx+lc*sp+gap;
  for(let i=0;i<rc;i++)pos.push(rx+i*sp);
  const tw=(pos.length?pos[pos.length-1]:sx)+60;
  svg.setAttribute('viewBox','0 0 '+tw+' 80');
  let h='<defs><linearGradient id="wg" x1="0" y1="0" x2="0" y2="1">';
  h+='<stop offset="0%" stop-color="#C4A035"/><stop offset="45%" stop-color="#9B7A1C"/>';
  h+='<stop offset="100%" stop-color="#6B4F10"/></linearGradient></defs>';
  h+='<rect x="15" y="18" width="'+(tw-30)+'" height="44" rx="22" fill="url(#wg)" stroke="#5C4A0A" stroke-width="2"/>';
  h+='<ellipse cx="24" cy="'+cy+'" rx="13" ry="24" fill="#B08C20" stroke="#5C4A0A" stroke-width="2"/>';
  for(let i=0;i<nf;i++){h+='<circle id="fh'+i+'" cx="'+pos[i]+'" cy="'+cy+'" r="'+r+'" class="flute-hole closed"/>'}
  if(lc>0&&rc>0){
    const sepX=pos[lc-1]+sp/2+gap/2;
    h+='<line x1="'+sepX+'" y1="24" x2="'+sepX+'" y2="56" stroke="#5C4A0A" stroke-width="1" stroke-dasharray="3,3" opacity="0.4"/>';
    const lcx=(pos[0]+pos[lc-1])/2,rcx=(pos[lc]+pos[nf-1])/2;
    h+='<text x="'+lcx+'" y="76" text-anchor="middle" class="flute-lbl">Main gauche</text>';
    h+='<text x="'+rcx+'" y="76" text-anchor="middle" class="flute-lbl">Main droite</text>';
  }
  svg.innerHTML=h;
}
function updateAir(nd){
  const s=$('airSlider'),sec=$('airSection');if(!s||!sec)return;
  if(nd){sec.classList.remove('air-off');
    $('airFill').style.left=nd.air_min+'%';$('airFill').style.width=(nd.air_max-nd.air_min)+'%';
    $('airMinL').textContent='Min: '+nd.air_min+'%';$('airMaxL').textContent='Max: '+nd.air_max+'%';
    s.value=Math.round((nd.air_min+nd.air_max)/2*1.27);onAirChange(s.value)}
  else{sec.classList.add('air-off');$('airPct').textContent='-'}
}
function onAirChange(v){
  $('airPct').textContent=Math.round(v/1.27)+'%';
  wsSend({t:'cc',c:2,v:parseInt(v)})
}
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
  const sel=$('calNoteSelect');if(sel){sel.innerHTML='';
  notesData.forEach(n=>{
    const o=document.createElement('option');o.value=n.midi;
    o.textContent=midiToName(n.midi)+' (MIDI '+n.midi+')';sel.appendChild(o);
  })}
  // Init airflow slider
  if(cfgData){$('calAirSlider').value=cfgData.air_off;$('calAirVal').textContent=cfgData.air_off}
  // Build mini flute for calibration visual feedback
  buildCalFlute();
  // Reset note cal panel
  calNoteIdx=-1;
  const panel=$('noteCalPanel');if(panel)panel.classList.remove('active');
}

function testFinger(idx,angle){
  $('calF'+idx+'Val').textContent=angle;
  wsSend({t:'test_finger',i:idx,a:parseInt(angle)});
  updateCalFlute();
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
  if(cfgData){testAirflow(cfgData.air_off);$('calAirSlider').value=cfgData.air_off}
  testSolenoid(0);
  if(sweepTimer){clearInterval(sweepTimer);sweepTimer=null}
}

// --- Calibration mini-flute (#4 visual feedback) ---
function buildCalFlute(){
  const svg=$('calFluteSvg');if(!svg)return;
  const nf=numFingers,sp=50,sx=80,gap=30,cy=40,r=14;
  const lc=Math.ceil(nf/2),rc=nf-lc;
  const pos=[];
  for(let i=0;i<lc;i++)pos.push(sx+i*sp);
  const rx=sx+lc*sp+gap;
  for(let i=0;i<rc;i++)pos.push(rx+i*sp);
  const tw=(pos.length?pos[pos.length-1]:sx)+60;
  svg.setAttribute('viewBox','0 0 '+tw+' 80');
  let h='<defs><linearGradient id="cwg" x1="0" y1="0" x2="0" y2="1">';
  h+='<stop offset="0%" stop-color="#C4A035"/><stop offset="45%" stop-color="#9B7A1C"/>';
  h+='<stop offset="100%" stop-color="#6B4F10"/></linearGradient></defs>';
  h+='<rect x="15" y="18" width="'+(tw-30)+'" height="44" rx="22" fill="url(#cwg)" stroke="#5C4A0A" stroke-width="2"/>';
  h+='<ellipse cx="24" cy="'+cy+'" rx="13" ry="24" fill="#B08C20" stroke="#5C4A0A" stroke-width="2"/>';
  for(let i=0;i<nf;i++){h+='<circle id="cfh'+i+'" cx="'+pos[i]+'" cy="'+cy+'" r="'+r+'" class="flute-hole closed"/>'}
  if(lc>0&&rc>0){
    const sepX=pos[lc-1]+sp/2+gap/2;
    h+='<line x1="'+sepX+'" y1="24" x2="'+sepX+'" y2="56" stroke="#5C4A0A" stroke-width="1" stroke-dasharray="3,3" opacity="0.4"/>';
  }
  svg.innerHTML=h;
}
function updateCalFlute(){
  if(!cfgData)return;
  const ao=cfgData.angle_open||30;
  for(let i=0;i<numFingers;i++){
    const h=$('cfh'+i);if(!h)continue;
    const slider=$('calF'+i);
    if(!slider)continue;
    const cur=parseInt(slider.value);
    const closed=cfgData.fingers[i].a;
    const isOpen=Math.abs(cur-closed)>ao/2;
    h.setAttribute('class','flute-hole '+(isOpen?'open':'closed'));
  }
}

// --- Calibration par note (#1) ---
function selectCalNote(){
  const midi=parseInt($('calNoteSelect').value);
  const nd=notesData.find(n=>n.midi===midi);
  const panel=$('noteCalPanel');
  if(!nd){if(panel)panel.classList.remove('active');calNoteIdx=-1;return}
  calNoteIdx=notesData.indexOf(nd);
  panel.classList.add('active');
  $('ncNoteName').textContent=midiToName(nd.midi);
  $('ncNoteInfo').textContent='MIDI '+nd.midi;
  $('ncAirMin').value=nd.air_min;$('ncAirMinVal').textContent=nd.air_min+'%';
  $('ncAirMax').value=nd.air_max;$('ncAirMaxVal').textContent=nd.air_max+'%';
  // Update cal flute to show this note fingering
  for(let i=0;i<numFingers;i++){
    const h=$('cfh'+i);if(!h)continue;
    h.setAttribute('class','flute-hole '+(nd.fingers&&nd.fingers[i]?'open':'closed'));
  }
  $('calFluteInfo').textContent=midiToName(nd.midi)+': '+nd.fingers.map(function(f){return f?'O':'\u25CF'}).join(' ');
  // Reset sweep
  $('sweepPanel').classList.remove('active');
}
function onNcAirChange(){
  const mn=parseInt($('ncAirMin').value),mx=parseInt($('ncAirMax').value);
  $('ncAirMinVal').textContent=mn+'%';$('ncAirMaxVal').textContent=mx+'%';
  // Live preview: send airflow at midpoint
  if(cfgData){
    const mid=(mn+mx)/2;
    const angle=Math.round(cfgData.air_min+(cfgData.air_max-cfgData.air_min)*mid/100);
    wsSend({t:'test_air',a:angle});
    $('calAirVal').textContent=angle;$('calAirSlider').value=angle;
  }
}
function testCalNote(){
  if(calNoteIdx<0)return;
  const nd=notesData[calNoteIdx];
  wsSend({t:'test_note',n:nd.midi});
  testSolenoid(1);
  $('calFluteInfo').textContent='Test: '+midiToName(nd.midi)+' (solenoide ouvert)';
}
function saveNoteCalibration(){
  if(calNoteIdx<0)return;
  notesData[calNoteIdx].air_min=parseInt($('ncAirMin').value);
  notesData[calNoteIdx].air_max=parseInt($('ncAirMax').value);
  const body={notes_air:notesData.map(function(n){return{mn:n.air_min,mx:n.air_max}})};
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(body)}).then(function(r){return r.json()}).then(function(d){
    if(d.ok){$('calFluteInfo').textContent='Sauvegarde OK !';addLog("Cal note sauvee");
      buildKeyboard();buildFingeringTable()}
  }).catch(function(e){$('calFluteInfo').textContent='Erreur: '+e});
}

// --- Sweep airflow (#5) ---
function startSweep(){
  if(calNoteIdx<0)return;
  testNote();testSolenoid(1);
  sweepPct=0;sweepMin=-1;sweepMax=-1;
  $('sweepPanel').classList.add('active');
  $('sweepRange').style.display='none';
  $('btnSweepMin').disabled=false;
  $('btnSweepMax').disabled=true;
  $('sweepPct').textContent='0%';
  $('sweepCursor').style.left='0%';
  if(sweepTimer)clearInterval(sweepTimer);
  sweepTimer=setInterval(function(){
    sweepPct+=1;
    if(sweepPct>100){
      clearInterval(sweepTimer);sweepTimer=null;
      testSolenoid(0);
      if(sweepMin>=0&&sweepMax<0)sweepMax=100;
      applySweepResult();return;
    }
    $('sweepPct').textContent=sweepPct+'%';
    $('sweepCursor').style.left=sweepPct+'%';
    if(sweepMin>=0){
      $('sweepRange').style.width=(sweepPct-sweepMin)+'%';
    }
    if(cfgData){
      const angle=Math.round(cfgData.air_min+(cfgData.air_max-cfgData.air_min)*sweepPct/100);
      wsSend({t:'test_air',a:angle});
    }
  },150);
}
function markSweepMin(){
  sweepMin=sweepPct;
  $('btnSweepMin').disabled=true;$('btnSweepMax').disabled=false;
  $('sweepRange').style.display='block';
  $('sweepRange').style.left=sweepPct+'%';$('sweepRange').style.width='0%';
}
function markSweepMax(){
  sweepMax=sweepPct;
  $('btnSweepMax').disabled=true;
  if(sweepTimer){clearInterval(sweepTimer);sweepTimer=null}
  testSolenoid(0);applySweepResult();
}
function applySweepResult(){
  if(sweepMin>=0&&sweepMax>sweepMin){
    $('ncAirMin').value=sweepMin;$('ncAirMinVal').textContent=sweepMin+'%';
    $('ncAirMax').value=sweepMax;$('ncAirMaxVal').textContent=sweepMax+'%';
    $('sweepRange').style.left=sweepMin+'%';
    $('sweepRange').style.width=(sweepMax-sweepMin)+'%';
    $('calFluteInfo').textContent='Sweep: '+sweepMin+'% - '+sweepMax+'%  (cliquez Sauver)';
  }
}

// --- Wizard calibration doigts (#2) ---
function startWizard(){
  wizStep=0;wizFingerData=[];
  for(let i=0;i<numFingers;i++){
    wizFingerData.push({closed:cfgData?cfgData.fingers[i].a:90,dir:cfgData?cfgData.fingers[i].d:1});
  }
  $('wizOverlay').classList.add('active');
  wizSetStep(0);
}
function wizSetStep(step){
  wizStep=step;
  const total=numFingers+1;
  let dots='';
  for(let i=0;i<total;i++){dots+='<span class="dot '+(i<step?'done':(i===step?'cur':''))+'"></span>'}
  $('wizProgress').innerHTML=dots;

  if(step<numFingers){
    const i=step;const angle=wizFingerData[i].closed;
    $('wizTitle').textContent='Doigt '+(i+1)+' / '+numFingers;
    $('wizStepInfo').textContent='Etape '+(step+1)+' / '+total;
    $('wizBody').innerHTML=
      '<div style="color:#e0e0e0;margin-bottom:14px;line-height:1.4">'+
      'Deplacez le slider jusqu\'a ce que le doigt <strong style="color:#e94560">couvre completement</strong> le trou '+(i+1)+'.</div>'+
      '<div style="text-align:center;margin:12px 0"><div style="font-size:2em;font-weight:bold;color:#4ecca3" id="wizAngleVal">'+angle+'</div>'+
      '<div style="font-size:0.75em;color:#888">degres</div></div>'+
      '<input type="range" class="cal-range" id="wizSlider" min="0" max="180" value="'+angle+
      '" oninput="wizSliderMove(this.value)">';
    wsSend({t:'test_finger',i:i,a:angle});
  }else{
    $('wizTitle').textContent='Validation';
    $('wizStepInfo').textContent='Etape '+total+' / '+total;
    let s='<div style="color:#e0e0e0;margin-bottom:14px;line-height:1.4">'+
      'Verifiez chaque doigt. Ajustez l\'amplitude d\'ouverture puis testez.</div>';
    s+='<div class="cal-servo"><div class="cal-label">Angle ouverture (tous doigts) '+
      '<span style="color:#4ecca3;float:right" id="wizOpenVal">'+(cfgData?cfgData.angle_open:30)+'</span></div>'+
      '<input type="range" class="cal-range" id="wizOpenSlider" min="5" max="90" value="'+(cfgData?cfgData.angle_open:30)+
      '" oninput="wizOpenChange(this.value)"></div>';
    s+='<div style="margin-top:10px">';
    for(let i=0;i<numFingers;i++){
      s+='<div class="wiz-finger-row">'+
        '<span>Doigt '+(i+1)+'</span>'+
        '<span class="wiz-angle">'+wizFingerData[i].closed+'&deg;</span>'+
        '<button class="btn-action" style="font-size:0.75em;padding:4px 8px" onclick="wizTestFinger('+i+',true)">Ouvrir</button>'+
        '<button class="btn-action" style="font-size:0.75em;padding:4px 8px" onclick="wizTestFinger('+i+',false)">Fermer</button>'+
        '<button class="btn-action" style="font-size:0.75em;padding:4px 8px" onclick="wizFlipDir('+i+')">Inverser</button></div>';
    }
    s+='</div>';
    $('wizBody').innerHTML=s;
  }
  $('wizPrev').style.display=step>0?'':'none';
  $('wizNext').textContent=step<numFingers?'Valider \u2192':'Sauvegarder';
  $('wizNext').onclick=step<numFingers?wizNext:wizFinish;
}
function wizSliderMove(v){
  wizFingerData[wizStep].closed=parseInt(v);
  $('wizAngleVal').textContent=v;
  wsSend({t:'test_finger',i:wizStep,a:parseInt(v)});
}
function wizNext(){wizSetStep(wizStep+1)}
function wizPrev(){if(wizStep>0)wizSetStep(wizStep-1)}
function wizTestFinger(i,open){
  const closed=wizFingerData[i].closed;
  const ao=parseInt($('wizOpenSlider')?$('wizOpenSlider').value:30);
  const dir=wizFingerData[i].dir;
  const angle=open?closed+ao*dir:closed;
  wsSend({t:'test_finger',i:i,a:Math.max(0,Math.min(180,angle))});
}
function wizOpenChange(v){$('wizOpenVal').textContent=v}
function wizFlipDir(i){
  wizFingerData[i].dir*=-1;
  addLog("Doigt "+(i+1)+": dir="+wizFingerData[i].dir);
}
function wizFinish(){
  const openSlider=$('wizOpenSlider');
  const body={
    angle_open:openSlider?parseInt(openSlider.value):30,
    fingers:wizFingerData.map(function(f){return{a:f.closed,d:f.dir}})
  };
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(body)}).then(function(r){return r.json()}).then(function(d){
    if(d.ok){
      $('wizOverlay').classList.remove('active');
      addLog("Wizard: calibration sauvegardee");
      loadConfig();
    }
  }).catch(function(e){addLog("Erreur wizard: "+e)});
}
function wizClose(){
  $('wizOverlay').classList.remove('active');
  resetCalibration();
}

// --- Visual fingering table (#3) ---
function buildFingeringTable(){
  const tb=$('fingeringBody');if(!tb)return;
  tb.innerHTML='';
  const lc=Math.ceil(numFingers/2);
  notesData.forEach(function(n,idx){
    const tr=document.createElement('tr');
    let dots='<div class="fing-dots-cell">';
    for(let i=0;i<numFingers;i++){
      if(i===lc)dots+='<span class="fing-sep"></span>';
      const open=n.fingers&&n.fingers[i]?true:false;
      dots+='<span class="fing-dot '+(open?'open':'closed')+'" data-ni="'+idx+'" data-fi="'+i+
        '" onclick="toggleFingerDot(this)"></span>';
    }
    dots+='</div>';
    tr.innerHTML='<td style="font-weight:bold;color:#e0e0e0">'+midiToName(n.midi)+'</td>'+
      '<td style="color:#888;font-size:0.8em">'+n.midi+'</td>'+
      '<td>'+dots+'</td>'+
      '<td><input type="number" id="nMin'+idx+'" min="0" max="100" value="'+n.air_min+'" style="width:48px"></td>'+
      '<td><input type="number" id="nMax'+idx+'" min="0" max="100" value="'+n.air_max+'" style="width:48px"></td>'+
      '<td><button class="btn-action" style="font-size:0.72em;padding:3px 6px" onclick="testTableNote('+idx+')">Test</button></td>';
    tb.appendChild(tr);
  });
}
function toggleFingerDot(el){
  const ni=parseInt(el.dataset.ni),fi=parseInt(el.dataset.fi);
  if(notesData[ni]&&notesData[ni].fingers){
    notesData[ni].fingers[fi]=notesData[ni].fingers[fi]?0:1;
    el.className='fing-dot '+(notesData[ni].fingers[fi]?'open':'closed');
  }
}
function testTableNote(idx){
  const n=notesData[idx];
  if(n)wsSend({t:'test_note',n:n.midi});
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
    // Construire UI dynamique
    buildFlute();
    buildKeyboard();
    buildCalibration();
    buildFingeringTable();
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
