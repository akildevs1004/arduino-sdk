File uploadFile;

void updateFirmwareDatasetup() {
}




void handleUpdatePage() {

  if (!isAuthenticated()) {
    server.sendHeader("Location", "/");
    server.send(302);
    return;
  }

  String header = readFile("/header.html");



  // Serve the update page
  String html = header + "<form method='POST' action='/updatefirmwaredatafilessubmit' enctype='multipart/form-data'>";
  html += "<input type='file' name='update'>";
  html += "<input type='submit' value='Update'>";
  html += "</form>";

    html=replaceHeaderContent(html);

  server.send(200, "text/html", html);
}

void handleFileUpload() {
  if (!isAuthenticated()) {
    server.sendHeader("Location", "/");
    server.send(302);
    return;
  }

  HTTPUpload& upload = server.upload();

  switch (upload.status) {
    case UPLOAD_FILE_START:
      {
        String filename = upload.filename;
        if (!filename.startsWith("/")) filename = "/" + filename;

        Serial.println("=== Upload Info ===");
        Serial.print("Filename: ");
        Serial.println(filename);
        Serial.println("Status: START");

        if (LittleFS.exists(filename)) {
          Serial.println("File exists. Overwriting...");
          LittleFS.remove(filename);
        } else {
          Serial.println("Creating new file...");
        }

        uploadFile = LittleFS.open(filename, "w");
        if (!uploadFile) {
          Serial.println("Failed to open file for writing");
        }
        break;
      }

    case UPLOAD_FILE_WRITE:
      {
        Serial.println("Status: WRITE");
        Serial.print("Chunk Size: ");
        Serial.println(upload.currentSize);

        if (uploadFile) {
          uploadFile.write(upload.buf, upload.currentSize);

          // Log buffer as text
          Serial.println("Buffer (text):");
          Serial.write(upload.buf, upload.currentSize);
          Serial.println();
        }
        break;
      }

    case UPLOAD_FILE_END:
      {
        Serial.println("Status: END");
        Serial.print("Total Size: ");
        Serial.println(upload.totalSize);
        Serial.println("===================");

        if (uploadFile) {
          uploadFile.close();
          server.send(200, "text/plain", "File upload complete!");
        } else {
          server.send(500, "text/plain", "Upload failed");
        }
        break;
      }

    case UPLOAD_FILE_ABORTED:
      {
        Serial.println("Upload aborted");
        break;
      }
  }
}



void handleNotFound() {

  if (!isAuthenticated()) {
    server.sendHeader("Location", "/");
    server.send(302);
    return;
  }
  // Check if the file exists in SPIFFS
  String path = server.uri();
  if (path.endsWith("/")) path += "index.html";

  String contentType = getContentType(path);

  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
  } else {
    server.send(404, "text/plain", "File Not Found");
  }
}

String getContentType(String filename) {
  // Determine content type based on file extension
  if (filename.endsWith(".html")) return "text/html";
  if (filename.endsWith(".css")) return "text/css";
  if (filename.endsWith(".js")) return "application/javascript";
  if (filename.endsWith(".png")) return "image/png";
  if (filename.endsWith(".jpg")) return "image/jpeg";
  if (filename.endsWith(".gif")) return "image/gif";
  if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}