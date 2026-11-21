#include <fstream>
#include "Ticket_Metrics.h"
#include <iostream>
#include <iomanip>

using namespace std;

ParkingTicket::ParkingTicket(string tId, string vId, string plate, int sId, int floor, int arr, int dep, double fee)
    : ticketId(tId), vehicleId(vId), licensePlate(plate), slotId(sId),
      floorNumber(floor), arrivalTime(arr), departureTime(dep), totalFee(fee) {}

string ParkingTicket::getTicketId() const { return ticketId; }
string ParkingTicket::getVehicleId() const
{
    return vehicleId;
}

string ParkingTicket::getLicensePlate() const
{
    return licensePlate;
}

int ParkingTicket::getSlotId() const
{
    return slotId;
}

int ParkingTicket::getFloorNumber() const
{
    return floorNumber;
}

int ParkingTicket::getArrivalTime() const
{
    return arrivalTime;
}

int ParkingTicket::getDepartureTime() const
{
    return departureTime;
}
double ParkingTicket::getFee() const { return totalFee; }

void ParkingTicket::printTicket() const {
    cout << "\n================ PARKING TICKET ================\n";
    cout << " Ticket ID     : " << ticketId << "\n";
    cout << " Vehicle ID    : " << vehicleId << " (" << licensePlate << ")\n";
    cout << " Assigned Slot : ID " << slotId << " (Floor " << floorNumber << ")\n";
    cout << " Duration      : " << arrivalTime << "m to " << departureTime << "m (" << (departureTime - arrivalTime) << " mins)\n";
    cout << " Total Bill    : $" << fixed << setprecision(2) << totalFee << "\n";
    cout << "================================================\n";
}

AdminMetricsManager::AdminMetricsManager() : cumulativeRevenue(0.0) {}

void AdminMetricsManager::recordTicket(const ParkingTicket& ticket) {
    ticketHistory.push_back(ticket);
    cumulativeRevenue += ticket.getFee();
}

double AdminMetricsManager::getTotalRevenue() const {
    return cumulativeRevenue;
}

double AdminMetricsManager::calculateOccupancyRate(const vector<ParkingSlot*>& slots) const {
    if (slots.empty()) return 0.0;
    
    int occupiedCount = 0;
    size_t i = 0;
    
    while (i < slots.size()) {
        const ParkingSlot* slot = slots[i];
        if (slot->getState() == STATE_OCCUPIED) {
            occupiedCount++;
        }
        i++;
    }
    
    return (static_cast<double>(occupiedCount) / slots.size()) * 100.0;
}

void AdminMetricsManager::displayAdminDashboard(const vector<ParkingSlot*>& slots) const {
    cout << "\n================ ADMIN METRICS DASHBOARD ================\n";
    cout << " Total Facility Revenue : $" << fixed << setprecision(2) << cumulativeRevenue << "\n";
    cout << " Total Tickets Processed: " << ticketHistory.size() << "\n";
    cout << " Current Occupancy Rate : " << calculateOccupancyRate(slots) << "%\n";
    cout << "=========================================================\n";
}

bool AdminMetricsManager::exportTicketsToCSV(const string &filename) const
{

    ofstream file(filename);

    if (!file.is_open())
    {
        cout << "Unable to open CSV file!\n";
        return false;
    }

    file << "TicketID,VehicleID,LicensePlate,SlotID,Floor,"
         << "ArrivalTime,DepartureTime,Duration,TotalFee\n";

    for (const ParkingTicket &ticket : ticketHistory)
    {

        int duration = ticket.getDepartureTime() -
                       ticket.getArrivalTime();

        file << ticket.getTicketId() << ","
             << ticket.getVehicleId() << ","
             << ticket.getLicensePlate() << ","
             << ticket.getSlotId() << ","
             << ticket.getFloorNumber() << ","
             << ticket.getArrivalTime() << ","
             << ticket.getDepartureTime() << ","
             << duration << ","
             << fixed << setprecision(2) << ticket.getFee()
             << "\n";
    }

    file.close();

    cout << "Tickets exported successfully!\n";
    return true;
}