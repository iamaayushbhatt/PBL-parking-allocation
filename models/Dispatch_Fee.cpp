#include "DispatchAndFee.h"

using namespace std;

ParkingSlot* DispatchStrategy::findBestSlot(Vehicle* v, vector<ParkingSlot*>& slots) {
    for (ParkingSlot* slot : slots) {
        if (slot->getState() != STATE_FREE) continue;

        if (v->requiresEV() && !slot->checkEVZone()) continue;
        if (v->requiresAccessible() && !slot->checkAccessibleZone()) continue;
        if (slot->getSizeRank() < v->getSizeRank()) continue;

        return slot;
    }
    return nullptr;
}

ParkingTicket FeeCalculator::generateTicket(string ticketId, Vehicle* v, ParkingSlot* slot, int departureTime) {
    int duration = departureTime - v->getArrivalTime();
    double baseRate = 1.5;
    double multiplier = 1.0;

    if (v->requiresEV()) {
        multiplier = 2.0;
    } else if (v->requiresAccessible()) {
        multiplier = 0.5;
    }

    if (v->getSize() == "LARGE") {
        multiplier *= 1.3;
    }

    double finalFee = duration * baseRate * multiplier;

    return ParkingTicket(
        ticketId,
        v->getVehicleId(),
        v->getLicensePlate(),
        slot->getId(),
        slot->getFloor(),
        v->getArrivalTime(),
        departureTime,
        finalFee
    );
}