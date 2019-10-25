#include <VAGRadioRemote.h>

#define REMOTE_PIN 2
#define REMOTE_OUT 3
VAGRadioRemote remote(REMOTE_OUT, REMOTE_PIN);

void setup() {
  remote.begin();
}

void loop() {
 if (remote.gotNewCode()) {
    remote.send((remote.newCode()));
    remote.clearGotNewCode();
  }
}
