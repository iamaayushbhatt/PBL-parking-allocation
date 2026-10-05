#include <string>
#include <iostream>

class Vehicle {
protected:
    std::string licensePlate;
    int arrivalTime;
    std::string size; 
    bool evRequired;
    bool accessibleRequired;
    std::string vehicleCategory;


public:
    Vehicle(std::string plate, int time, std::string vSize, bool isEV, bool isAcc, std::string category)
    : licensePlate(plate), arrivalTime(time), size(vSize), evRequired(isEV), accessibleRequired(isAcc), vehicleCategory(category) {}


    std::string getLicensePlate() const { return licensePlate; }
    int getArrivalTime() const { return arrivalTime; }
    std::string getSize() const { return size; }
    bool requiresEV() const { return evRequired; }
    bool requiresAccessible() const { return accessibleRequired; }
    std::string getVehicleCategory() const { return vehicleCategory; }
};


class ElectricVehicle : public Vehicle {
public:
    ElectricVehicle(std::string plate, int time, std::string vSize)
     : Vehicle(plate, time, vSize, true, false, "Electric") {}
};

class AccessibleVehicle : public Vehicle {
public:
    AccessibleVehicle(std::string plate, int time, std::string vSize)
     : Vehicle(plate, time, vSize, false, true, "Accessible") {}
};

class StandardVehicle : public Vehicle {
public:
    StandardVehicle(std::string plate, int time, std::string vSize)
        : Vehicle(plate, time, vSize, false, false, "Standard") {}
};