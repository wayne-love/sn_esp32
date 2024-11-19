#include "Spa.h"

Spa::Spa() : port(SPA_SERIAL) {
    SPA_SERIAL.setRxBufferSize(1024);  //required for unit testing
    SPA_SERIAL.setTxBufferSize(1024);  //required for unit testing
    SPA_SERIAL.begin(38400, SERIAL_8N1, RX_PIN, TX_PIN);
    SPA_SERIAL.setTimeout(250);

    STMP.setBeforeUpdate(this, &Spa::setSTMP);

}

void Spa::sendCommand(String cmd) {

    clearSerialReadBuffer();

    debugD("Sending - %s",cmd.c_str());
    port.print('\n');
    port.flush();
    delay(50); // **TODO** is this needed?
    port.printf("%s\n", cmd.c_str());
    port.flush();

    ulong timeout = millis() + 1000; // wait up to 1 sec for a response

    debugD("Start waiting for a response");
    while (port.available()==0 and millis()<timeout) {}
    debugD("Finish waiting");

    _resultRegistersDirty = true; // we're trying to write to the registers so we can assume that they will now be dirty
}

String Spa::sendCommandReturnResult(String cmd) {
    sendCommand(cmd);
    String result = port.readStringUntil('\r');
    port.read(); // get rid of the trailing LF char
    debugV("Read - %s",result.c_str());
    return result;
}

bool Spa::sendCommandCheckResult(String cmd, String expected){
    String result = sendCommandReturnResult(cmd);
    bool outcome = result == expected;
    if (!outcome) debugW("Sent comment %s, expected %s, got %s",cmd.c_str(),expected.c_str(),result.c_str());
    return outcome;
}

void Spa::clearSerialReadBuffer() {
    int bytesFlushed = 0;

    debugD("Flushing serial stream - %i bytes in the buffer", port.available());
    while (port.available() > 0 && bytesFlushed++<5120) { 
        int byteRead = port.read();
        debugV("%i,",byteRead);
    }
    debugD("Flushed serial stream - %i bytes in the buffer", port.available());
}

bool Spa::setSTMP(int temp) {
    debugD("setSTMP - %i", temp);
    String stemp = String(temp);
    return sendCommandCheckResult("W40:" + stemp, stemp);
}