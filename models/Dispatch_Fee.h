#pragma once
#include <vector>
#include <string>
#include "VehicleHierarchy.h"
#include "Slot_State.h"
#include "Ticket_Metrics.h"

using std::vector;
using std::string;

class DispatchStrategy {
public:
    ParkingSlot* findBestSlot(Vehicle* v, vector<ParkingSlot*>& slots);
};

class FeeCalculator {
public:
    ParkingTicket generateTicket(string ticketId, Vehicle* v, ParkingSlot* slot, int departureTime);
};