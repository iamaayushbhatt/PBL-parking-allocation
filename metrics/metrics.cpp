#include "metrics.h"
#include <iostream>
#include <iomanip>

// Grams of CO2 per metre driven.
// Standard / Accessible: 122 g/km, the fleet average of new passenger cars in
//   India in FY 2019-20 (ICCT, "Fuel consumption from new passenger cars in
//   India", 2021: 122.4 g/km). Accessible vehicles are ordinary cars, so they
//   use the same value.
// Electric: 0, because an EV has no exhaust. Electricity emissions at the
//   power plant are not counted here (stated assumption).
double Metrics::emissionFactorFor(const std::string &category)
{
    if (category == "Electric")
        return 0.0;
    return 0.122; // Standard, Accessible and anything unknown
}

double Metrics::baselineDistance(const std::vector<double> &distancesToFreeSlots)
{
    if (distancesToFreeSlots.empty())
        return 0;
    double sum = 0;
    for (double d : distancesToFreeSlots)
        sum += d;
    return sum / distancesToFreeSlots.size();
}

void Metrics::recordArrival(const std::string &vehicleId, double time)
{
    Record r;
    r.arrivalTime = time;
    r.parkedTime = 0;
    r.exitTime = 0;
    r.distanceDriven = 0;
    r.baselineDistance = 0;
    r.emissionFactor = 0;
    r.parked = false;
    r.exited = false;
    records[vehicleId] = r;
}

bool Metrics::recordParked(const std::string &vehicleId, double time, double distanceDriven,
                           double baselineDistance, const std::string &category)
{
    auto it = records.find(vehicleId);
    if (it == records.end() || it->second.parked)
        return false;

    Record &r = it->second;
    r.parkedTime = time;
    r.distanceDriven = distanceDriven;
    r.baselineDistance = baselineDistance;
    r.emissionFactor = emissionFactorFor(category);
    r.parked = true;
    return true;
}

bool Metrics::recordExit(const std::string &vehicleId, double time)
{
    auto it = records.find(vehicleId);
    if (it == records.end() || !it->second.parked || it->second.exited)
        return false;

    it->second.exitTime = time;
    it->second.exited = true;
    return true;
}

double Metrics::timeToPark(const std::string &vehicleId) const
{
    auto it = records.find(vehicleId);
    if (it == records.end() || !it->second.parked)
        return -1;
    return it->second.parkedTime - it->second.arrivalTime;
}

double Metrics::parkingDuration(const std::string &vehicleId) const
{
    auto it = records.find(vehicleId);
    if (it == records.end() || !it->second.exited)
        return -1;
    return it->second.exitTime - it->second.parkedTime;
}

double Metrics::emissions(const std::string &vehicleId) const
{
    auto it = records.find(vehicleId);
    if (it == records.end() || !it->second.parked)
        return -1;
    return it->second.distanceDriven * it->second.emissionFactor;
}

double Metrics::emissionsSaved(const std::string &vehicleId) const
{
    auto it = records.find(vehicleId);
    if (it == records.end() || !it->second.parked)
        return -1;
    const Record &r = it->second;
    return (r.baselineDistance - r.distanceDriven) * r.emissionFactor;
}

int Metrics::arrivedCount() const
{
    return (int)records.size();
}

int Metrics::parkedCount() const
{
    int n = 0;
    for (const auto &entry : records)
        if (entry.second.parked)
            n++;
    return n;
}

int Metrics::exitedCount() const
{
    int n = 0;
    for (const auto &entry : records)
        if (entry.second.exited)
            n++;
    return n;
}

double Metrics::averageTimeToPark() const
{
    double sum = 0;
    int n = 0;
    for (const auto &entry : records)
    {
        const Record &r = entry.second;
        if (r.parked)
        {
            sum += r.parkedTime - r.arrivalTime;
            n++;
        }
    }
    return n == 0 ? 0 : sum / n;
}

double Metrics::averageParkingDuration() const
{
    double sum = 0;
    int n = 0;
    for (const auto &entry : records)
    {
        const Record &r = entry.second;
        if (r.exited)
        {
            sum += r.exitTime - r.parkedTime;
            n++;
        }
    }
    return n == 0 ? 0 : sum / n;
}

double Metrics::totalEmissions() const
{
    double sum = 0;
    for (const auto &entry : records)
    {
        const Record &r = entry.second;
        if (r.parked)
            sum += r.distanceDriven * r.emissionFactor;
    }
    return sum;
}

double Metrics::totalEmissionsSaved() const
{
    double sum = 0;
    for (const auto &entry : records)
    {
        const Record &r = entry.second;
        if (r.parked)
            sum += (r.baselineDistance - r.distanceDriven) * r.emissionFactor;
    }
    return sum;
}

void Metrics::printReport(std::ostream &out) const
{
    out << std::fixed << std::setprecision(2);
    out << "===== Simulation Metrics =====\n";
    out << "Vehicles arrived        : " << arrivedCount() << "\n";
    out << "Vehicles parked         : " << parkedCount() << "\n";
    out << "Vehicles exited         : " << exitedCount() << "\n";
    out << "Avg time to park        : " << averageTimeToPark() << "\n";
    out << "Avg parking duration    : " << averageParkingDuration() << "\n";
    out << "Total emissions (g CO2) : " << totalEmissions() << "\n";
    out << "Emissions saved (g CO2) : " << totalEmissionsSaved() << "\n";
}