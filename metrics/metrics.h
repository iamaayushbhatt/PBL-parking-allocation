#ifndef METRICS_H
#define METRICS_H

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

// Collects per-vehicle timing and distance data reported by the simulation
// and turns it into time-to-park, parking duration and emissions figures.
//
// Units are whatever the simulation uses:
//   times     -> same unit as Event::timestamp
//   distances -> same unit as the Graph's lane distances
//   emissions -> grams of CO2 (we assume 1 distance unit = 1 metre)
class Metrics
{
private:
    struct Record
    {
        double arrivalTime;
        double parkedTime;
        double exitTime;
        double distanceDriven;
        double baselineDistance; // distance driven WITHOUT the smart routing
        double emissionFactor; // grams of CO2 per distance unit
        bool parked;
        bool exited;
    };

    std::map<std::string, Record> records; // key = vehicleId

public:
    // grams of CO2 per metre for a vehicle category ("Standard", "Accessible",
    // "Electric"). Unknown categories are treated as "Standard".
    static double emissionFactorFor(const std::string &category);

    // The baseline: how far a driver WITHOUT guidance would drive. We assume
    // such a driver ends up at a random suitable free slot, so the baseline is
    // the average distance from the entry to every suitable free slot.
    // The orchestrator passes in those distances (0 if the list is empty).
    static double baselineDistance(const std::vector<double> &distancesToFreeSlots);

    // --- the orchestrator reports events here ---
    void recordArrival(const std::string &vehicleId, double time);

    // distanceDriven    = Graph::pathDistance of the route actually taken
    // baselineDistance  = see baselineDistance() above
    // category          = vehicle category, used to pick the emission factor
    // false if the vehicle never arrived or was already parked
    bool recordParked(const std::string &vehicleId, double time,
                      double distanceDriven, double baselineDistance,
                      const std::string &category);

    // false if the vehicle is not currently parked
    bool recordExit(const std::string &vehicleId, double time);

    // --- per-vehicle results (-1 if not known yet) ---
    double timeToPark(const std::string &vehicleId) const;
    double parkingDuration(const std::string &vehicleId) const;
    double emissions(const std::string &vehicleId) const;
    double emissionsSaved(const std::string &vehicleId) const; // can be negative

    // --- totals and averages ---
    int arrivedCount() const;
    int parkedCount() const;
    int exitedCount() const;
    double averageTimeToPark() const;
    double averageParkingDuration() const;
    double totalEmissions() const;
    double totalEmissionsSaved() const;

    void printReport(std::ostream &out) const;
};

#endif
