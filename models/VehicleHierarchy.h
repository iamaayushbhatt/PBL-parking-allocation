#pragma once
#include <string>

using std::string;

class Vehicle {
protected:
    string vehicleId;
    string licensePlate;
    int arrivalTime;
    string size;
    bool evRequired;
    bool accessibleRequired;
    string vehicleCategory;

public:
    Vehicle(string id, string plate, int time, string vSize, bool isEV, bool isAcc, string category);

    string getVehicleId() const;
    string getLicensePlate() const;
    int getArrivalTime() const;
    string getSize() const;
    int getSizeRank() const;
    bool requiresEV() const;
    bool requiresAccessible() const;
    string getVehicleCategory() const;
};

class ElectricVehicle : public Vehicle {
public:
    ElectricVehicle(string id, string plate, int time, string vSize);
};

class AccessibleVehicle : public Vehicle {
public:
    AccessibleVehicle(string id, string plate, int time, string vSize);
};

class StandardVehicle : public Vehicle {
public:
    StandardVehicle(string id, string plate, int time, string vSize);
};