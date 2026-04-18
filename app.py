from flask import Flask, render_template, request, jsonify
import socket

app = Flask(__name__)

# =====================================================================
# ⚠️ EDIT THESE IP ADDRESSES BEFORE RUNNING ⚠️
# =====================================================================
ESP32_MAIN_IP = "10.30.175.76"  # The IP of your Main ESP32
ESP32_PORT = 4210

# The IP Webcam app typically broadcasts on port 8080.
# Make sure to keep "/video" at the end of the URL to get the raw stream!
MOBILE_CAM_URL = "http://10.30.175.100:8080/video" 

udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

@app.route('/')
def index():
    # Renders the HTML UI and passes the mobile camera URL to it
    return render_template('index.html', cam_url=MOBILE_CAM_URL)

@app.route('/cmd', methods=['POST'])
def send_command():
    data = request.json
    cmd_type = data.get('type')
    cmd_string = ""
    
    # Format the data into comma-separated strings for the ESP32 to parse
    if cmd_type == "MOTOR":
        cmd_string = f"MOTOR,{data['left']},{data['right']}"
    elif cmd_type == "ARM":
        cmd_string = f"ARM,{data['b']},{data['s']},{data['e']},{data['c']}"
    elif cmd_type == "MODE":
        cmd_string = f"MODE,{data['mode']}"
        
    if cmd_string:
        try:
            udp_sock.sendto(cmd_string.encode(), (ESP32_MAIN_IP, ESP32_PORT))
        except Exception as e:
            print(f"Network Error: {e}")
            
    return jsonify({"status": "Success"})

if __name__ == '__main__':
    # Runs the web server locally on your Mac on port 5000
    app.run(host='0.0.0.0', port=5000, debug=True)