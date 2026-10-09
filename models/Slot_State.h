#pragma once
#include <string>
#include "VehicleHierarchy.h"

using std::string;

extern const string STATE_FREE;
extern const string STATE_OCCUPIED;
extern const string STATE_RESERVED;
extern const string STATE_MAINTENANCE;

class ParkingSlot {
private:
    int slotId;
    int floorNumber;
    string slotSize;
    string currentState;
    bool isEVZone;
    bool isAccessibleZone;
    Vehicle* currentVehicle;

public:
    ParkingSlot(int id, int floor, string size, bool ev, bool acc);

    void setState(const string& newState);
    bool occupySlot(Vehicle* v);
    void vacateSlot();

    string getState() const;
    int getId() const;
    int getFloor() const;
    string getSlotSize() const;
    int getSizeRank() const;
    bool checkEVZone() const;
    bool checkAccessibleZone() const;
    Vehicle* getOccupyingVehicle() const;
};