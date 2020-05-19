# Copyright (C) 2020  Christian Berger
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Part to build wgs84togpx.
FROM alpine:3.11.3 as builder
MAINTAINER Christian Berger "christian.berger@gu.se"
RUN apk update && \
    apk --no-cache add \
        cmake \
        g++ \
        linux-headers \
        make \
        wget \
        upx
ADD . /opt/sources
WORKDIR /opt/sources
RUN cd /tmp && wget https://raw.githubusercontent.com/joubu/gpx2png/master/gpx2png.pl
RUN mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/tmp/wgs84togpx-dest .. && \
    make && make install && upx -9 /tmp/wgs84togpx-dest/bin/wgs84togpx


# Part to deploy wgs84togpx.
FROM alpine:3.11.3
MAINTAINER Christian Berger "christian.berger@gu.se"
RUN apk update && \
    apk --no-cache add \
        perl \
        imagemagick \
        imagemagick-perlmagick \
        perl-libwww
 
WORKDIR /usr/bin
COPY --from=builder /tmp/gpx2png.pl .
COPY --from=builder /tmp/wgs84togpx-dest/bin/wgs84togpx .
ADD wgs84topng.sh /usr/bin/
ENTRYPOINT ["/usr/bin/wgs84topng.sh"]

