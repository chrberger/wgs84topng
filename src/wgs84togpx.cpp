/*
 * Copyright (C) 2020  Christian Berger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cluon-complete.hpp"
#include "opendlv-standard-message-set.hpp"

#include <cmath>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

int32_t main(int32_t argc, char **argv) {
    int32_t retCode{0};
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if ( (0 == commandlineArguments.count("in")) || (0 == commandlineArguments.count("out")) ) {
        std::cerr << argv[0] << " converts GPS to GPX" << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --in=<existing recording> --out=<output>" << std::endl;
        std::cerr << "Example: " << argv[0] << " --in=myRec.rec --out=myNewRec.gpx" << std::endl;
        retCode = 1;
    } else {
        constexpr bool AUTOREWIND{false};
        constexpr bool THREADING{false};
        cluon::Player player(commandlineArguments["in"], AUTOREWIND, THREADING);
        if (!player.hasMoreData()) {
            std::cerr << "Failed to open in file." << std::endl;
            return retCode;
        }

        std::fstream fout(commandlineArguments["out"], std::ios::out|std::ios::trunc);
        if (!fout.good()) {
            std::cerr << "Failed to open out file." << std::endl;
            return retCode;
        }

        cluon::OD4Session od4{99};
        std::stringstream sstr;
        const char *_HEADER = R"(<?xml version="1.0" encoding="UTF-8"?>
<gpx version="1.0">
	<name>Example gpx</name>
	<trk><name>Example gpx</name><number>1</number><trkseg>
)";
        std::string header(_HEADER);
        sstr << header;
        uint32_t elementCounter{0};
        time_t timestamp{0};
        while (player.hasMoreData() && od4.isRunning()) {
            auto next = player.getNextEnvelopeToBeReplayed();
            if (next.first) {
                cluon::data::Envelope e = next.second;

                if (e.dataType() == opendlv::proxy::GeodeticWgs84Reading::ID()) {
                    if (timestamp != e.sampleTimeStamp().seconds()) {
                        timestamp = e.sampleTimeStamp().seconds();
                        struct tm  ts;
                        char buf[80];

                        // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
                        ts = *localtime(&timestamp);
                        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &ts);
                        std::string t(buf);
                        opendlv::proxy::GeodeticWgs84Reading pos = cluon::extractMessage<opendlv::proxy::GeodeticWgs84Reading>(std::move(e));
                        if ( ((pos.latitude() < 0) || pos.latitude() > 0) &&
                             ((pos.longitude() < 0) || pos.longitude() > 0) ) {
                             elementCounter++;
                             sstr << "<trkpt lat=\"" << std::setprecision(8) << pos.latitude() << "\" lon=\"" << pos.longitude() << "\">"
                                  << "<ele>" << 0 << "</ele><time>" << t << "</time></trkpt>" << std::endl;
                        }
                        //<trkpt lat="46.57608333" lon="8.89241667"><ele>2376</ele><time>2007-10-14T10:09:57Z</time></trkpt>
                    }
                }
            }
        }
        sstr << std::endl
        << "</trkseg></trk>" << std::endl
        << "</gpx>" << std::endl;
        fout << sstr.str();
        fout.flush();
        fout.close();
    }
    return retCode;
}
