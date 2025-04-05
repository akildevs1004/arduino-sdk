const net = require("net");
const express = require("express");
const fs = require("fs");
const path = require("path");
const axios = require("axios");
const app = express();
const api_port = 6000; // HTTP API port
const socket_port = 6002; // Socket port
app.use(express.json()); // This line is crucial for parsing JSON bodies
// Get today's date in 'YYYY-MM-DD' format
const today = new Date().toISOString().split("T")[0]; // e.g., "2024-09-11"
const logFilePath = path.join(__dirname, `${today}.log`);

// Override console.log to write to both console and file
const logStream = fs.createWriteStream(logFilePath, { flags: "a" }); // Append mode
const originalConsoleLog = console.log;

console.log = (...args) => {
  const logMessage = args
    .map((a) => (typeof a === "object" ? JSON.stringify(a) : a))
    .join(" ");
  const timestamp = new Date().toISOString();

  // Log to console
  originalConsoleLog.apply(console, args);

  // Log to file with timestamp
  logStream.write(`[${timestamp}] ${logMessage}\n`);
};

// In-memory storage
const deviceConfigs = {}; // { serialNumber: { socket, config_data, last_heartbeat } }

// Socket server for ESP32 devices
const server = net.createServer((socket) => {
  console.log("Server Running");

  let buffer = "";

  socket.on("data", (data) => {
    buffer += data.toString();

    let boundary = buffer.indexOf("\n"); // Assuming newline-delimited messages
    while (boundary > -1) {
      const message = buffer.substring(0, boundary);
      ////////console.log("Message : ", message);
      buffer = buffer.substring(boundary + 1);

      try {
        const dataObject = JSON.parse(message);
        const serialNumber = dataObject.serialNumber;

        // Store socket for this device
        deviceConfigs[serialNumber] = deviceConfigs[serialNumber] || {};
        deviceConfigs[serialNumber].socket = socket;
        deviceConfigs[serialNumber].last_heartbeat = new Date().toISOString();
        deviceConfigs[serialNumber].config_data = dataObject.config;
        if (dataObject.type != "heartbeat")
          console.log("Received config from device:", serialNumber);

        //console.log(dataObject.type);
        console.log(
          new Date().toISOString(),
          "Received Message from device:",
          serialNumber,
          dataObject.type
        );

        // if (dataObject.type == "switchStatus") {
        //   console.log(dataObject.switchStatus);
        //   const postData = {
        //     room_number: serialNumber,
        //     status: dataObject.switchStatus,
        //   };

        //   axios
        //     .post(
        //       "https://backend.ezhms.com/api/update_device_room_fill_status",
        //       postData
        //     )
        //     .then((response) => {
        //       console.log("Response data:", response.data);
        //     })
        //     .catch((error) => {
        //       console.error("Error posting data:", message, error.message);
        //     });
        // }

        if (dataObject.type === "heartbeat") {
          // Handle heartbeat data
          deviceConfigs[serialNumber].last_heartbeat = new Date().toISOString();
          console.log("Received heartbeat from device:", serialNumber);
        } else if (dataObject.type === "config") {
          // Handle configuration data
          deviceConfigs[serialNumber].config_data = dataObject.config;
          console.log("Received config from device:", serialNumber);
        }
      } catch (err) {
        console.error("Error parsing message:", err);
      }

      boundary = buffer.indexOf("\n");
    }
  });

  socket.on("end", () => {
    console.log("ESP32 disconnected");
  });

  //   socket.on("error", (err) => {
  //     console.error("SDK Socket error:", err);
  //   });
  socket.on("error", (err) => {
    if (err.code === "ECONNRESET") {
      console.warn("Client connection was reset");
    } else {
      console.error("Socket error:", err);
    }
  });
});

server.listen(socket_port, "0.0.0.0", () => {
  console.log(`SDK server listening on port ${socket_port}`);
});

// Express HTTP API for PHP
app.get("/device-config/:serialNumber", (req, res) => {
  const serialNumber = req.params.serialNumber;
  const deviceData = deviceConfigs[serialNumber];

  if (deviceData) {
    // Check if the device is still connected by checking the socket
    if (deviceData.socket && !deviceData.socket.destroyed) {
      const message = {
        action: "GET_CONFIG",
        serialNumber: serialNumber,
      };

      // Send the message to the device
      deviceData.socket.write(JSON.stringify(message) + "\n");

      console.log("Config Message sent to Device");
    } else res.status(404).json({ error: "Device SDK is not responding" });
    res.json({
      serialNumber: serialNumber,
      config: deviceData.config_data || {},
      last_heartbeat: deviceData.last_heartbeat || "No heartbeat received",
    });
  } else {
    res
      .status(404)
      .json({ error: "Device not available or Communication issue" });
  }
});
// POST route to update device config
app.post("/device-config-update/:serialNumber", (req, res) => {
  console.log("device-config-update");
  const serialNumber = req.params.serialNumber;
  console.log("Request From Device ", serialNumber);
  let newConfig = req.body.config;

  newConfig.lastUpdated = new Date().toISOString();

  const deviceData = deviceConfigs[serialNumber];

  if (deviceData) {
    // Check if the device is still connected by checking the socket
    if (deviceData.socket && !deviceData.socket.destroyed) {
      if (!newConfig) {
        console.log("Invalid or missing config data");
        return res
          .status(400)
          .json({ error: "Invalid or missing config data" });
      }
      deviceData.socket.write(JSON.stringify({ Hello: "Hello" }) + "\n");
      // Send the updated config to the device
      const message = {
        action: "UPDATE_CONFIG",
        serialNumber: serialNumber,
        config: newConfig,
      };
      //deviceData.socket.write(JSON.stringify(message) + "\n");
      // Write the message to the device socket
      try {
        deviceData.socket.write(JSON.stringify(message) + "\n");
        console.log(`Sent updated config to device ${serialNumber}:`, message);
        res.json({
          success: true,
          message: `Config sent to device ${serialNumber}`,
        });
      } catch (error) {
        console.error(`Error sending config to device ${serialNumber}:`, error);
        res.status(500).json({ error: "Failed to send config to device" });
      }
    } else {
      console.log("Device SDK not found or not connected");

      return res
        .status(404)
        .json({ error: "Device not found or not connected" });
    }
  } else {
    console.log("Device not found or not connected");

    return res.status(404).json({ error: "Device not found or not connected" });
  }
});
app.listen(api_port, () => {
  console.log(`API server listening at http://localhost:${api_port}`);
});
