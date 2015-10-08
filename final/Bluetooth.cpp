#include "Bluetooth.h"

#define TEAM_NUMBER 16

Bluetooth::Bluetooth(){ //Initializes the Bluetooth object
  
  this->setup();
  //BluetoothClient bt;
  //BluetoothMaster btmaster;
}

/** a callable function that takes care of the Bluetooth setup
 */
void Bluetooth::setup()
{
  this->sendHB = false;
  this->Go = false;
  this->teamName = 16;
  this->radLevel = 0; 
  this->pcol = ReactorProtocol(byte(TEAM_NUMBER));
  
  Serial3.begin(115200); //Serial3 for the Mega
  Timer1.initialize(1000*100); //Triggers every 100 ms 
  Timer1.attachInterrupt(timerISR);
  pcol.setDst(0x00); //Always set to broadcast to everyone //FIXME
  elapsedTics = 0; //sets the elapsedTics

}
/** a callable function that takes care of reading the Bluetooth and taking care of the relevant assigns
 */
void Bluetooth::update(){
	
  if(btmaster.readPacket(pktR)){ //If there is a packet to read
	  //TODO: create constants instead of numbers for array index
    if(pcol.getData(pktR, dataR, type) && (dataR[4] == TEAM_NUMBER || dataR[4] == 0x00)){ //If it's addressed to us or all
      switch (type) { //state machine for the types of packets to be read
        case STORAGE:
          updateStorage(dataR[0]); //updates the storage bools with the relevant info
          break;
        case SUPPLY:
          updateSupply(dataR[0]); //updates the supply bools with the relevant info
          break;
        case STOP:
          this->Go = false; //sets flags for movement 
          break;
        case RESUME:
          this->Go = true;
          break;
      }
    }
  }
}



/** Updates the storage struct with the relevant booleans
 * @param info the information byte that is read from Bluetooth
 */
void Bluetooth::updateStorage(byte info){
  storageTube->tube1 = bitread(info, 0); //if LSB is on, sets the bool in storageTube
  storageTube->tube2 = bitread(info, 1);
  storageTube->tube3 = bitread(info, 2);
  storageTube->tube4 = bitread(info, 3);
}

/** Updates the supply tubes with the relevant booleans
 * @param info the information byte that was read 
 */
void Bluetooth::updateSupply(byte info){
  supplyTube->tube1 = bitread(info, 0); //if LSB is on, sets the bool in supplyTube
  supplyTube->tube2 = bitread(info, 1);
  supplyTube->tube3 = bitread(info, 2);
  supplyTube->tube4 = bitread(info, 3);
}

/** sends the Heartbeat
 * needs to be wrapped in a timed statement so as not to spam 
 */ 
void Bluetooth::sendHB(){
  this->sendHB? = false;
  szS = pcol.createPkt(HEARTBEAT, dataS, pktS); //creates the packt
  enqueue(Qs, &szS); //stores the packet info and the size
  enqueue(Qp, &pktS);
}

/** Sends the radiation level
 * @param radLevel what radiation level needs to be sent
 */ 
void Bluetooth::sendRadiation(int radLevel){
  if(radLevel == 1){ //determines radiation and sets the data correctly
    data[0] = 0x2C;
  }else if(radLevel == 2){
    data[0] = 0xFF;
  }
  szS = pcol.createPkt(RADIATION, dataS, pktS);//creates and enqueues the packet
  enqueue(Qs, &szS);
  enqueue(Qp, &pktS);
}

/** Sets the Flag and sends next packet upon proper time
 */
void Bluetooth::timerISR(){
  noInterrupts(); //stops the interrupts 
  elapsedTics++; //increments the tics
  if(elapsedTics % 20){ //every 2s
    this->sendHB? = true; //set the flag
  } else if ((elapsedTics + 5) % 10){ //every 1 sec (offset from HB by 0.5s)
    sendNxtPkt(); //send the next packet in the queue
  }
  interrupts(); //reenables 
}
/** Creates a sends a packet with information about movement, gripper, and opstat
 * @param moveStat the movement status, reference enum
 * @param gripStat the gripper status, reference enum
 * @param opStat the operation status, reference enum
 */
void Bluetooth::sendStatus(byte moveStat, byte gripStat, byte opStat){
  dataS[0] = moveStat; //sets the correct data bytes to the information given
  dataS[1] = gripStat;
  dataS[2] = opStat;
  szS = pcol.createPkt(STATUS, dataS, pktS); //creates and enqueues the info
  enqueue(Qs, &szS);
  enqueue(Qp, &pktS);
}
/** creates a Queue with max entries as stated
 * @param max_entries the maximum size of the array
 * @return the Queue struct created
 */
Queue Bluetooth::createQueue(int max_entries){
  //TODO
}

/**adds an array to the Queue
 * @param Q the queue to add the array to
 * @param pkt the pkt pointer to be added
 * @return int 0 for success, -1 for failure (too full)
 */
int Bluetooth::enqueue(Queue Q, byte* pkt){
  //TODO
}
/** takes the first entry of the Q and returns it 
 * @param Q the queue to dequeue
 * @return the pointer that was dequeued
 */
byte* Bluetooth::dequeue(Queue Q){
  //TODO
}
/** dequeus the next packet and sends it 
 */
void sendNxtPkt(){
  //TODO dequeue relevant info
  btmaster.sendPkt(pkt, sz);
}
