#include "VehicleHierarchy.h"

using namespace std;

Vehicle::Vehicle(string id, string plate, int time, string vSize, bool isEV, 
    bool isAcc, string category)
    : vehicleId(id), licensePlate(plate), arrivalTime(time), size(vSize),
      evRequired(isEV), accessibleRequired(isAcc), vehicleCategory(category) {}

string Vehicle::getVehicleId() const { return vehicleId; }
string Vehicle::getLicensePlate() const { return licensePlate; }
int Vehicle::getArrivalTime() const { return arrivalTime; }
string Vehicle::getSize() const { return size; }

int Vehicle::getSizeRank() const {
    if (size == "COMPACT") return 1;
    if (size == "STANDARD") return 2;
    if (size == "LARGE") return 3;
    return 1;
}

bool Vehicle::requiresEV() const { return evRequired; }
bool Vehicle::requiresAccessible() const { return accessibleRequired; }
string Vehicle::getVehicleCategory() const { return vehicleCategory; }

ElectricVehicle::ElectricVehicle(string id, string plate, int time, string vSize)
    : Vehicle(id, plate, time, vSize, true, false, "Electric") {}

AccessibleVehicle::AccessibleVehicle(string id, string plate, int time, string vSize)
    : Vehicle(id, plate, time, vSize, false, true, "Accessible") {}

StandardVehicle::StandardVehicle(string id, string plate, int time, string vSize)
    : Vehicle(id, plate, time, vSize, false, false, "Standard") {}