#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>

using namespace std;
using namespace boost;
using namespace boost::asio;
// using namespace LibSerial;

#define PAGE_SIZE 0x100

uint32_t char2hex (char c) {
    if ((c >= '0') && (c <= '9')) return c - '0';
    if ((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
    BOOST_VERIFY(0);
    return 0;
}

char hex2char (uint8_t v) {
    if (v < 10) return v + '0';
    if (v < 16) return v - 10 + 'A';
    BOOST_VERIFY(0);
    return 0;
}

uint8_t char2hex (char c1, char c2) {
    return uint8_t((char2hex(c1) << 4) + char2hex(c2));
}

uint16_t char2hex (char c1, char c2, char c3, char c4) {
    return uint16_t((char2hex(c1) << 12) + (char2hex(c2) << 8) + (char2hex(c3) << 4) + char2hex(c4));
}

class HEX: public vector<uint8_t> {
public:
    HEX (string const &path, size_t S = 0x2000)
    {
        resize(S);
        fill(begin(), end(), 0xFF);
        ifstream is(path.c_str());
        char c1, c2, c3, c4;
        for (;;) {
            for (;;) {
                is >> c1;
                BOOST_VERIFY(is);
                if (c1 == ':') break;
            }
            is >> c1 >> c2;
            uint8_t n = char2hex(c1, c2);
            is >> c1 >> c2 >> c3 >> c4;
            uint16_t addr = char2hex(c1, c2, c3, c4);
            is >> c1 >> c2;
            uint8_t type = char2hex(c1, c2);
            if (type == 1) {
                BOOST_VERIFY(n == 0);
                BOOST_VERIFY(addr == 0);
                break;
            }
            BOOST_VERIFY(type == 0);
            for (uint8_t i = 0; i < n; ++i) {
                is >> c1 >> c2;
                uint8_t v = char2hex(c1, c2);
                at(addr + i) = v;
            }
            is >> c1 >> c2;
        }
        size_t sz = size();
        while ((sz > 0) && (at(sz-1) == 0xFF)) {
            --sz;
        }
        sz = (sz + 0xFF) / PAGE_SIZE * PAGE_SIZE;
        resize(sz);
    }
};

class delayed_serial {
    io_service io;
    serial_port port;
    int delay;
public:
    delayed_serial (string const &dev, int baud, int char_size, int parity, int stop_bits, int flow_control, int delay_)
        : port(io, dev), delay(delay_)
    {
        port.set_option(serial_port_base::baud_rate(baud));
        BOOST_VERIFY(port.is_open());
        port.set_option(serial_port_base::character_size(char_size));
        port.set_option(serial_port_base::parity(serial_port_base::parity::type(parity)));
        port.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::type(stop_bits)));
        port.set_option(serial_port_base::flow_control(serial_port_base::flow_control::type(flow_control)));
    }

    void write (void const *_p, size_t s) {
        char const *p = reinterpret_cast<char const *>(_p);
        for (size_t i = 0; i < s; ++i, ++p) {
            boost::asio::write(port, const_buffers_1(p, 1));
            usleep(delay);
        }
    }

    void fast_write (void const *p, size_t s) {
        boost::asio::write(port, const_buffers_1(p, s));
    }

    void write (char const *p) {
        for (; *p; ++p) {
            boost::asio::write(port, const_buffers_1(p, 1));
            usleep(delay);
        }
    }

    void write (int p) {
        char c = p;
        boost::asio::write(port, const_buffers_1(&c, 1));
        usleep(delay);
    }

    int read () {
        char c;
        boost::asio::read(port, buffer(&c, 1));
        return c;
    }

    void read (void *p, size_t s) {
        boost::asio::read(port, buffer(p, s));
    }
};

int main (int argc, char *argv[]) {
    string input;
    string dev;
    int baud;
    int char_size = 8;
    int parity;
    int stop_bits;
    int flow_control;
    int delay;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message.")
        ("input,i", po::value(&input), "")
        ("port,s", po::value(&dev)->default_value("/dev/ttyUSB0"), "")
        ("baud", po::value(&baud)->default_value(9600), "")
        // ("char-size", po::value(&char_size)->default_value(8), "")
        ("parity", po::value(&parity)->default_value(0), "0: none, 1: odd, 2:even")
        ("stop-bits", po::value(&stop_bits)->default_value(0), "0: one, 1: onepointfive, 2: two")
        ("flow-control", po::value(&flow_control)->default_value(0), "0: none, 1: software, 2: hardware")
        ("delay", po::value(&delay)->default_value(1000))
        ("verify", "")
        ("page", "")
        ("dump", "");

    po::positional_options_description p;
    p.add("input", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
                     options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help") || (vm.count("input") == 0)) {
        cout << "Usage:" << endl;
        cout << desc;
        cout << endl;
        return 0;
    }

    HEX data(input);

    if (vm.count("dump")) {
dump:
        for (size_t i = 0; i < data.size(); ++i) {
            cout << hex2char(data[i] >> 4) << hex2char(data[i] & 0xF);
            if ((i % 2) == 0) continue;
            if ((i % 16) == 15) {
                cout << endl;
            }
            else {
                cout << ' ';
            }
        }
        return 0;
    }

    uint8_t page = 0;

    if (vm.count("page")) page = 1;

    delayed_serial serial(dev, baud, char_size, parity, stop_bits, flow_control, delay);

    serial.write("hello51");

    if (vm.count("verify")) {
        serial.write(2);    // method
        serial.write(page);
        int r;
        cerr << "writing size ..." << endl;
        int c = data.size() / PAGE_SIZE;
        serial.write(c);
        for (int i = 0; i < c; ++i) {
            cerr << "reading " << i << '/' << c << "..." << endl;
            serial.read(&data[i * PAGE_SIZE], PAGE_SIZE);
            uint8_t checksum = 0;
            for (int j = 0; j < PAGE_SIZE; ++j) {
                checksum = checksum ^ data[i * PAGE_SIZE + j];
            }
            r = serial.read();
            // BOOST_VERIFY(r == checksum);
        }
        goto dump;
    }
    else {
        serial.write(1);    // method
        serial.write(page);
        int r = serial.read();
        BOOST_VERIFY(r == 0);
        cerr << "writing size ..." << endl;
        int c = data.size() / PAGE_SIZE;
        serial.write(c);
        r = serial.read();
        BOOST_VERIFY(r == 0);
        for (int i = 0; i < c; ++i) {
            cerr << "writing " << i << '/' << c << "..." << endl;
            serial.write(&data[i * PAGE_SIZE], PAGE_SIZE);
            char checksum = 0;
            for (int j = 0; j < PAGE_SIZE; ++j) {
                checksum = checksum ^ data[i * PAGE_SIZE + j];
            }
            r = serial.read();
            // BOOST_VERIFY(r == checksum);
        }
    }

    return 0;
}
