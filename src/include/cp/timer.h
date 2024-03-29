/*
 * Copyright 2014 MarrowTeam
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 *  March 2014: Herve Paulino
 *      Specification and implementation of the timer class (from pre-existent classes)
 *
 */

#ifndef MARROW_UTILS_TIMER_HPP
#define MARROW_UTILS_TIMER_HPP

#include <chrono>
#include <cmath>
#include <map>
#include <memory>
#include <iostream>
#include <vector>


namespace marrow {

    static const std::string main_stage;

    template <class Duration = std::chrono::milliseconds>
    class timer {

        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using ElapseTime = typename Duration::rep;


        struct stage {

            /**
             * Measurements
             */
            std::vector<ElapseTime> measurements;

            /**
             * Start of next measurement
             */
            TimePoint start;

            stage() :
                    measurements (),
                    start (std::forward<TimePoint>(Clock::now())),
                    average_computed_for (1),
                    stddev_computed_for (1),
                    number_measurements (0)
            {}


            // Auxiliary variables for computing averages and standard deviations
            unsigned average_computed_for;
            unsigned stddev_computed_for;
            unsigned number_measurements;

            ElapseTime average;
            double  stddev;

            unsigned account_from;
            unsigned account_to;

        };


    public:

        /**
         *
         * @param percentage Percentage of the middle measurements to use in the statistics.
         * For instance, for percentage = 90, the lower and upper 5% and not considered in the statistics.
         * Values <= 0 or > 100 are ignored.
         */
        explicit timer(const unsigned char percentage = 100) :
                percentage (percentage > 100 ? 100 : percentage)
        { }

        /**
         * Start a new measurement
         * @param stage_name Name of the stage: the default is main stage denoted by "_"
         */
        void start(const std::string& stage_name = main_stage) {
            auto s = stages.find(stage_name);
            if (s == stages.end())
                stages.emplace(stage_name, stage{});
            else
                s->second.start = Clock::now();
        }

        double stop(const std::string& stage_name = main_stage) {
            auto now = Clock::now();

            timer::stage &s = stages.at(stage_name);
            auto elapsed = std::chrono::duration_cast<Duration>(now - s.start).count();
            s.measurements.push_back(elapsed);

            return elapsed;
        }

        void insert_time(const Duration t, const std::string& stage_name = main_stage) {
            timer::stage* s;
            auto f = stages.find(stage_name);
            if (f == stages.end())
                s = &stages.emplace(stage_name, stage{}).first->second;
            else
                s = &f->second;

            s->measurements.push_back(t.count());
        }

        void reset(const std::string& name = main_stage) {
            timer::stage &s = stages.at(name);
            s.measurements.clear();
            s.average_computed_for = 0;
            s.stddev_computed_for = 0;
        }

        template <bool Incremental = false>
        ElapseTime average(const std::string& stage_name = main_stage) {
            stage &s = stages.at(stage_name);

            if constexpr (!Incremental) {
                if (s.average_computed_for == s.number_measurements) // already computed
                    return s.average;
            }

            auto n_measurements = s.measurements.size();
            if (n_measurements <= 1)
                return s.measurements[0];

            qsort(s.measurements.data(), n_measurements, sizeof(ElapseTime), compare);

            if (percentage < 100) {
                s.number_measurements = round(n_measurements * percentage / 100);
                s.account_from = s.number_measurements == n_measurements ?
                                 0 :
                                 round((n_measurements - s.number_measurements) / 2 - 1);
                s.account_to = s.account_from + s.number_measurements;
            }
            else {
                s.number_measurements = n_measurements;
                s.account_from = 0;
                s.account_to = s.number_measurements-1;
            }

            ElapseTime average = 0;
            for (unsigned i = s.account_from; i <= s.account_to; i++)
                average += s.measurements[i];
            average /= s.number_measurements;

            if constexpr (!Incremental) {
                s.average_computed_for = s.number_measurements;
                s.average = average;
            }

            return average;
        }


        template <bool Incremental = false>
        double std_deviation(const std::string& stage_name = main_stage) {
            stage &s = stages.at(stage_name);

            if constexpr (!Incremental) {
                if (s.stddev_computed_for == s.number_measurements) // already computed
                    return s.stddev;
            }

            auto n_measurements = s.measurements.size();
            if (n_measurements <= 1)
                return 0;

            const auto avg = average<Incremental>(stage_name);

            double variance = 0.0;
            for (unsigned int i = s.account_from; i <= s.account_to; i++) {
                auto aux = s.measurements[i] - avg;
                variance += aux * aux;
            }

            variance /= s.number_measurements;
            const auto stddev = std::sqrt(variance);
            
            if constexpr (!Incremental) {
                s.stddev_computed_for = s.number_measurements;
                s.stddev = stddev;
            }

            return stddev;
        }

        ElapseTime totaltime(const std::string& stage_name = main_stage) {
            stage &s = stages.at(stage_name);

            auto n_measurements = s.measurements.size();
            if (n_measurements <= 1)
                return s.measurements[0];

            qsort(s.measurements.data(), n_measurements, sizeof(ElapseTime), compare);

            if (percentage < 100) {
                s.number_measurements = round(n_measurements * percentage / 100);
                s.account_from = s.number_measurements == n_measurements ?
                                 0 :
                                 round((n_measurements - s.number_measurements) / 2 - 1);
                s.account_to = s.account_from + s.number_measurements;
            }
            else {
                s.number_measurements = n_measurements;
                s.account_from = 0;
                s.account_to = s.number_measurements-1;
            }

            ElapseTime totalTime = 0;
            for (unsigned i = s.account_from; i <= s.account_to; i++)
                totalTime += s.measurements[i];

            return totalTime;
        }

        template<bool CSV = false>
        void output_stats(std::ostream& out,
                         const std::string& stage_name = main_stage) {

            stage &s = stages.at(stage_name);

            if (s.measurements.size() <= 1)
                out << stage_name <<"\tTotal time:\t" << s.measurements[0] << std::endl;

            else {
                average(stage_name);

                if (CSV) {

                    out << s.number_measurements << "/" << s.measurements.size() <<
                        ", " << s.measurements[s.account_from] <<
                        ", " << s.measurements[s.account_to] <<
                        ", " << average(stage_name) <<
                        ", " << std_deviation(stage_name) << std::endl;
                } else {
                    out << stage_name << " statistics (middle " << s.number_measurements << " of "
                        << s.measurements.size() <<
                        " measurements) in " << ":" << std::endl <<
                        "\tTotal time:\t" << totaltime(stage_name) << std::endl <<
                        "\tAverage:\t" << average(stage_name) << std::endl <<
                        "\tMaximum:\t" << s.measurements[s.account_to] << std::endl <<
                        "\tMinimum:\t" << s.measurements[s.account_from] << std::endl <<
                        "\tStandard deviation: " << std_deviation(stage_name) << std::endl;
                }
            }
        }

        unsigned get_number_measurements(const std::string& stage_name = main_stage) {
            return stages.at(stage_name).measurements.size();
        }

    private:

        /**
         * Map of stage identifiers into stages
         */
        std::map<std::string, stage> stages;

        /**
         * Percentage of the middle measurements to use in the statistics
         */
        const unsigned char percentage;

        /**
         * Auxiliary compare function
         */
        static int compare(const void *a, const void *b) {
            if ( *(ElapseTime *)a <  *(ElapseTime *)b ) return -1;
            if ( *(ElapseTime *)a == *(ElapseTime *)b ) return 0;

            return 1;
        }
    };
}

#endif// MARROW_UTILS_TIMER_HPP


