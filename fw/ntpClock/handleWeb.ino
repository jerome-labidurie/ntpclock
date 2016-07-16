void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  String header = "<html><head><title>NTPClock</title></head><body><h1>NTPClock config</h1>";
  String footer = "</body></html>";

  if (server.hasArg("reset") ) {
    Serial.println("reset");
    wicoResetWifiConfig (1);
  } else if (server.hasArg("msg")) {
    if (server.hasArg("loop")) {
      // switch to MODE_MSG, loop on displaying a msg
      displayMode = MODE_MSG;
      String s = server.arg(0);
      s.toLowerCase();
      strncpy (currentMsg, s.c_str(), 255);
      currentMsg[255] = 0;
    } else {
      if (server.arg("msg") == "") {
        //switch to clock mode
        displayMode = MODE_CLOCK;
      } else {
        // only display a message
        //Serial.println(server.arg(0).c_str());
        String s = server.arg("msg");
        s.toLowerCase();
        strncpy(temp, s.c_str(), 400);
        if (s.length() > 5) {
          wScroll(temp);
        } else {
          wChaine(temp);
        }
      }
    }
  } else if (server.hasArg("bright")) {
    // set global brightness
    brightness = atoi(server.arg(0).c_str());
    ba.adapt = server.hasArg("adapt");
    ecIBright(255);
    ecGBright( brightness );
  } else if (server.hasArg("dts")) {
    // set DTS
    dts = atoi(server.arg(0).c_str());
    EEPROM.write(0, dts);
    EEPROM.commit();
    Serial.print("UTC+"); Serial.println(dts);
  }

  // wifi form
  int n = WiFi.scanNetworks();
  String s = header;
  s += "<p><form>Reset wifi <input type='checkbox' name='reset'><input type='submit' value='send'></form></p>\n";

  // Dayligth Time Saving
  s += "<p><form>UTC+<input type=text name=dts value='";
  s += dts;
  s += "'><input type=submit value=set></form></p>";

  // message form
  s += "<p><form>msg:<input type=text name=msg><input type=submit value=disp><input type=checkbox name=loop>loop?</form>\n";

  // brightness form
  s += "<form><input type='range'  min='0' max='255' name=bright onchange='showValue(this.value)' step='5' value='";
  s += brightness;
  s += "'/><input type=checkbox name=adapt ";
  s += ba.adapt==true?"checked":"";
  s += "><input type=submit value=bright></form>\n<span id='range'>0</span><script type='text/javascript'>function showValue(newValue){  document.getElementById('range').innerHTML=newValue;}</script>";

  snprintf ( temp, 400, "<pre>Uptime: %02d:%02d:%02d\n\
URI:%s\n\
Method:%s\n\
Args:%d\n</pre>",
    hr, min % 60, sec % 60,
    server.uri().c_str(), ( server.method() == HTTP_GET ) ? "GET" : "POST",server.args()
  );
  s += temp;
  s += footer;
  
  server.send ( 200, "text/html", s );
}

void handleNotFound() {
  String message = "Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  server.send ( 404, "text/plain", message );
}

