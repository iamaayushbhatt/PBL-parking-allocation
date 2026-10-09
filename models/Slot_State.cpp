#include "Slot_State.h"

using namespace std;

const string STATE_FREE = "FREE";
const string STATE_OCCUPIED = "OCCUPIED";
const string STATE_RESERVED = "RESERVED";
const string STATE_MAINTENANCE = "MAINTENANCE";

ParkingSlot::ParkingSlot(int id, int floor, string size, bool ev, bool acc)
    : slotId(id), floorNumber(floor), slotSize(size), currentState(STATE_FREE),
      isEVZone(ev), isAccessibleZone(acc), currentVehicle(nullptr) {}

void ParkingSlot::setState(const string& newState) {
    currentState = newState;
}

bool ParkingSlot::occupySlot(Vehicle* v) {
    if (currentState == STATE_FREE && v != nullptr) {
        currentVehicle = v;
        currentState = STATE_OCCUPIED;
        return true;
    }
    return false;
}

void ParkingSlot::vacateSlot() {
    currentVehicle = nullptr;
    currentState = STATE_FREE;
}

string ParkingSlot::getState() const { return currentState; }
int ParkingSlot::getId() const { return slotId; }
int ParkingSlot::getFloor() const { return floorNumber; }
string ParkingSlot::getSlotSize() const { return slotSize; }

int ParkingSlot::getSizeRank() const {
    if (slotSize == "COMPACT") return 1;
    if (slotSize == "STANDARD") return 2;
    if (slotSize == "LARGE") return 3;
    return 1;
}

bool ParkingSlot::checkEVZone() const { return isEVZone; }
bool ParkingSlot::checkAccessibleZone() const { return isAccessibleZone; }
Vehicle* ParkingSlot::getOccupyingVehicle() const { return currentVehicle; }