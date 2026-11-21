#pragma once
#include <string>
#include <vector>
#include "Slot_State.h"

using std::string;
using std::vector;

class ParkingTicket {
private:
    string ticketId;
    string vehicleId;
    string licensePlate;
    int slotId;
    int floorNumber;
    int arrivalTime;
    int departureTime;
    double totalFee;

public:
    ParkingTicket(string tId, string vId, string plate, int sId, int floor, int arr, int dep, double fee);

    string getTicketId() const;
    string getVehicleId() const;
    string getLicensePlate() const;
    int getSlotId() const;
    int getFloorNumber() const;
    int getArrivalTime() const;
    int getDepartureTime() const;
    double getFee() const;
    void printTicket() const;
};

class AdminMetricsManager {
private:
    vector<ParkingTicket> ticketHistory;
    double cumulativeRevenue;

public:
    AdminMetricsManager();
    bool exportTicketsToCSV(const string &filename) const;
    void recordTicket(const ParkingTicket& ticket);
    double getTotalRevenue() const;
    double calculateOccupancyRate(const vector<ParkingSlot*>& slots) const;
    void displayAdminDashboard(const vector<ParkingSlot*>& slots) const;
};