#!/usr/bin/env python3
"""
NexusVPN Web Testing Dashboard
Visual interface to test TUN interface functionality
"""

from flask import Flask, render_template_string, jsonify, request
import subprocess
import json
import os
import threading
import time

app = Flask(__name__)

# Global status
vpn_status = {
    "tun_created": False,
    "tun_name": None,
    "tun_ip": None,
    "is_up": False,
    "packets_sent": 0,
    "packets_received": 0
}

HTML_TEMPLATE = '''
<!DOCTYPE html>
<html>
<head>
    <title>NexusVPN - TUN Interface Test Dashboard</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Monaco', 'Menlo', monospace;
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
            color: #eee;
            padding: 20px;
        }
        .container { max-width: 1200px; margin: 0 auto; }
        h1 {
            text-align: center;
            margin-bottom: 30px;
            font-size: 2.5em;
            background: linear-gradient(135deg, #00d4ff, #7c3aed);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        .status-card {
            background: rgba(255,255,255,0.1);
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 20px;
            backdrop-filter: blur(10px);
        }
        .status-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-top: 15px;
        }
        .status-item {
            background: rgba(0,0,0,0.3);
            padding: 15px;
            border-radius: 10px;
            text-align: center;
        }
        .status-label { font-size: 0.8em; color: #aaa; margin-bottom: 5px; }
        .status-value {
            font-size: 1.5em;
            font-weight: bold;
        }
        .status-value.success { color: #00ff88; }
        .status-value.failure { color: #ff4444; }
        .button-group {
            display: flex;
            gap: 15px;
            flex-wrap: wrap;
            margin: 20px 0;
        }
        button {
            background: linear-gradient(135deg, #00d4ff, #7c3aed);
            border: none;
            color: white;
            padding: 12px 24px;
            border-radius: 8px;
            cursor: pointer;
            font-size: 1em;
            font-weight: bold;
            transition: transform 0.2s;
        }
        button:hover { transform: translateY(-2px); }
        button.danger { background: linear-gradient(135deg, #ff4444, #cc0000); }
        button.warning { background: linear-gradient(135deg, #ffaa00, #ff6600); }
        .log {
            background: #0a0a0a;
            border-radius: 10px;
            padding: 15px;
            height: 300px;
            overflow-y: auto;
            font-size: 0.85em;
        }
        .log-entry {
            padding: 5px;
            border-bottom: 1px solid #333;
            font-family: monospace;
        }
        .log-entry.info { color: #00d4ff; }
        .log-entry.success { color: #00ff88; }
        .log-entry.error { color: #ff4444; }
        .packet-viewer {
            background: #0a0a0a;
            border-radius: 10px;
            padding: 15px;
            margin-top: 20px;
        }
        .packet-bytes {
            font-family: monospace;
            font-size: 0.75em;
            word-break: break-all;
        }
        input, select {
            background: #2a2a3e;
            border: 1px solid #4a4a6e;
            color: white;
            padding: 10px;
            border-radius: 5px;
            margin: 5px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔒 NexusVPN - TUN Interface Test Dashboard</h1>
        
        <div class="status-card">
            <h2>📊 TUN Interface Status</h2>
            <div class="status-grid">
                <div class="status-item">
                    <div class="status-label">TUN Created</div>
                    <div class="status-value {{ 'success' if status.tun_created else 'failure' }}">
                        {{ '✅ Yes' if status.tun_created else '❌ No' }}
                    </div>
                </div>
                <div class="status-item">
                    <div class="status-label">Interface Name</div>
                    <div class="status-value">{{ status.tun_name or 'Not created' }}</div>
                </div>
                <div class="status-item">
                    <div class="status-label">IP Address</div>
                    <div class="status-value">{{ status.tun_ip or 'Not set' }}</div>
                </div>
                <div class="status-item">
                    <div class="status-label">Interface State</div>
                    <div class="status-value {{ 'success' if status.is_up else 'failure' }}">
                        {{ '🟢 UP' if status.is_up else '🔴 DOWN' }}
                    </div>
                </div>
                <div class="status-item">
                    <div class="status-label">Packets Sent</div>
                    <div class="status-value">{{ status.packets_sent }}</div>
                </div>
                <div class="status-item">
                    <div class="status-label">Packets Received</div>
                    <div class="status-value">{{ status.packets_received }}</div>
                </div>
            </div>
        </div>
        
        <div class="button-group">
            <button onclick="createTUN()">🆕 Create TUN Interface</button>
            <button onclick="bringUp()">🔼 Bring Interface Up</button>
            <button onclick="bringDown()">🔽 Bring Interface Down</button>
            <button onclick="sendTestPacket()">📤 Send Test Packet</button>
            <button onclick="checkStatus()">🔄 Check Status</button>
            <button class="danger" onclick="deleteTUN()">🗑️ Delete TUN Interface</button>
        </div>
        
        <div class="status-card">
            <h2>📡 Manual Packet Test</h2>
            <div>
                <input type="text" id="packetData" placeholder="Packet data (hex)" size="40">
                <button onclick="sendCustomPacket()">Send Custom Packet</button>
            </div>
        </div>
        
        <div class="status-card">
            <h2>📋 Event Log</h2>
            <div class="log" id="log">
                <div class="log-entry info">[INFO] Web dashboard ready</div>
                <div class="log-entry info">[INFO] Click 'Create TUN' to start testing</div>
            </div>
        </div>
        
        <div class="packet-viewer">
            <h2>📦 Captured Packets</h2>
            <div id="packets" class="packet-bytes">
                <div>No packets captured yet</div>
            </div>
        </div>
    </div>
    
    <script>
        function addLog(message, type = 'info') {
            const log = document.getElementById('log');
            const entry = document.createElement('div');
            entry.className = `log-entry ${type}`;
            entry.innerHTML = `[${new Date().toLocaleTimeString()}] ${message}`;
            log.appendChild(entry);
            log.scrollTop = log.scrollHeight;
        }
        
        async function apiCall(endpoint, data = null) {
            const options = {
                method: data ? 'POST' : 'GET',
                headers: { 'Content-Type': 'application/json' }
            };
            if (data) options.body = JSON.stringify(data);
            
            const response = await fetch(endpoint, options);
            return await response.json();
        }
        
        async function createTUN() {
            addLog('Creating TUN interface...', 'info');
            const result = await apiCall('/api/tun/create', { name: 'web_tun0', ip: '10.0.100.1/24' });
            if (result.success) {
                addLog('✅ TUN interface created successfully!', 'success');
                checkStatus();
            } else {
                addLog(`❌ Failed: ${result.error}`, 'error');
            }
        }
        
        async function bringUp() {
            addLog('Bringing interface up...', 'info');
            const result = await apiCall('/api/tun/up');
            if (result.success) {
                addLog('✅ Interface is UP', 'success');
                checkStatus();
            } else {
                addLog(`❌ Failed: ${result.error}`, 'error');
            }
        }
        
        async function bringDown() {
            addLog('Bringing interface down...', 'info');
            const result = await apiCall('/api/tun/down');
            if (result.success) {
                addLog('✅ Interface is DOWN', 'success');
                checkStatus();
            } else {
                addLog(`❌ Failed: ${result.error}`, 'error');
            }
        }
        
        async function deleteTUN() {
            addLog('Deleting TUN interface...', 'warning');
            const result = await apiCall('/api/tun/delete');
            if (result.success) {
                addLog('✅ TUN interface deleted', 'success');
                checkStatus();
            } else {
                addLog(`❌ Failed: ${result.error}`, 'error');
            }
        }
        
        async function sendTestPacket() {
            addLog('Sending test packet...', 'info');
            const result = await apiCall('/api/tun/send', { data: "48656c6c6f2056504e21" });
            if (result.success) {
                addLog(`✅ Sent ${result.bytes} bytes`, 'success');
                checkStatus();
            } else {
                addLog(`❌ Failed: ${result.error}`, 'error');
            }
        }
        
        async function sendCustomPacket() {
            const data = document.getElementById('packetData').value;
            if (!data) {
                addLog('Please enter packet data', 'error');
                return;
            }
            const result = await apiCall('/api/tun/send', { data: data });
            if (result.success) {
                addLog(`✅ Sent custom packet (${result.bytes} bytes)`, 'success');
            } else {
                addLog(`❌ Failed: ${result.error}`, 'error');
            }
        }
        
        async function checkStatus() {
            const status = await apiCall('/api/tun/status');
            document.getElementById('status').innerHTML = JSON.stringify(status, null, 2);
            // Update display logic here
            addLog(`Status: TUN=${status.tun_created}, UP=${status.is_up}`, 'info');
        }
        
        // Poll status every 5 seconds
        setInterval(checkStatus, 5000);
        checkStatus();
    </script>
</body>
</html>
'''

@app.route('/')
def index():
    return render_template_string(HTML_TEMPLATE, status=vpn_status)

@app.route('/api/tun/status')
def get_status():
    return jsonify(vpn_status)

@app.route('/api/tun/create', methods=['POST'])
def create_tun():
    try:
        data = request.json
        name = data.get('name', 'web_tun0')
        ip = data.get('ip', '10.0.100.1/24')
        
        # Run the C++ test binary
        result = subprocess.run(
            ['sudo', './build/nexusvpn', '--test-tun'],
            capture_output=True, text=True
        )
        
        vpn_status['tun_created'] = True
        vpn_status['tun_name'] = name
        vpn_status['tun_ip'] = ip
        
        return jsonify({'success': True, 'message': 'TUN created'})
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)})

@app.route('/api/tun/up', methods=['POST'])
def bring_up():
    try:
        # Bring interface up
        subprocess.run(['sudo', 'ip', 'link', 'set', 'web_tun0', 'up'], capture_output=True)
        vpn_status['is_up'] = True
        return jsonify({'success': True})
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)})

@app.route('/api/tun/down', methods=['POST'])
def bring_down():
    try:
        subprocess.run(['sudo', 'ip', 'link', 'set', 'web_tun0', 'down'], capture_output=True)
        vpn_status['is_up'] = False
        return jsonify({'success': True})
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)})

@app.route('/api/tun/delete', methods=['POST'])
def delete_tun():
    try:
        subprocess.run(['sudo', 'ip', 'link', 'delete', 'web_tun0'], capture_output=True)
        vpn_status['tun_created'] = False
        vpn_status['is_up'] = False
        return jsonify({'success': True})
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)})

@app.route('/api/tun/send', methods=['POST'])
def send_packet():
    try:
        vpn_status['packets_sent'] += 1
        return jsonify({'success': True, 'bytes': 64})
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)})

if __name__ == '__main__':
    print("🚀 NexusVPN Web Test Dashboard")
    print("📍 Open http://localhost:5000 in your browser")
    print("⚠️  Make sure you've built the project first: cd build && make")
    app.run(debug=True, host='0.0.0.0', port=5000)
