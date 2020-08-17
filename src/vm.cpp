#include "../inc/vm.hpp"

#if defined(_DEBUG) || defined(DEBUG)
#define Q_DEBUG
#endif

#include <xtr1common> // std::enable_if, std::is_arithmetic, std::remove_reference...

namespace vm { /* conversion utilities */

	using float32_t = float;
	using float64_t = double;

#define ARITHMETIC(tp)                                     \
	std::enable_if_t<                                      \
		std::is_arithmetic_v<std::remove_reference_t<tp>>, \
		std::remove_reference_t<tp>                        \
	>* = nullptr

	template <class src_t, class dst_t, ARITHMETIC(src_t), ARITHMETIC(dst_t)>
	dst_t to_type(const src_t val) {
		union {
			src_t s;
			dst_t d;
		} v;
		v.s = val;
		return v.d;
	}

}

#include <iostream> // std::cout, std::cin, std::cerr
#include <cctype>

namespace vm { /* hex utilities */

	uint8_t hex_dig_to_dec_dig(uint8_t val) {
		if (val >= '0' && val <= '9') { // [0-9]
			return val - '0';
		}
		if ((val = toupper(val)) >= 'A' && val <= 'F') { // [A-F]
			return val - 'A' + 10;
		}
		return 0;
	}

	uint8_t dec_val_to_hex_dig(uint8_t val) {
		if (val >= 0 && val <= 9) {
			return val + '0';
		} else if (val >= 10 && val <= 15) {
			return (val - 10) + 'A';
		}
		return 0;
	}

#define INTEGRAL(tp)                                     \
	std::enable_if_t<                                    \
		std::is_integral_v<std::remove_reference_t<tp>>, \
		std::remove_reference_t<tp>                      \
	>* = nullptr

	template <class type_t, INTEGRAL(type_t)>
	std::string to_hex(type_t val) {
		uint8_t len(sizeof(type_t));
		
		uint8_t* var = new uint8_t[len]{ 0 }; // val as array of u8
		for (uint8_t idx(0); idx < len; ++idx) {
			var[idx] = val >> (((len - 1) - idx) * 8);
		}

		std::string ret;
		uint8_t tmp[2]{ 0 }; // string in format 00
		for (uint8_t idx(0); idx < len; ++idx) {
			uint8_t dig(var[idx] / 0x10); // digit to parse
			tmp[0] = dec_val_to_hex_dig(dig);
			dig = var[idx] - (dig * 0x10);
			tmp[1] = dec_val_to_hex_dig(dig);
			ret.push_back(tmp[0]);
			ret.push_back(tmp[1]);
		}
		delete[] var;
		return std::string(ret);
	}

}

#include <fstream> // std::ifstream, std::ofstream, std::ios_base

namespace vm { /* file */

	std::string read(std::string path, std::ios_base::openmode mode) {
		std::ifstream in(path, mode);
		if (in.is_open()) {
			std::string ret;
			while (!in.eof()) {
				ret.push_back(in.get());
			}
			return ret;
		}
		return std::string();
	}

	void write(std::string path, std::ios_base::openmode mode, const std::string val) {
		std::ofstream out(path, mode);
		if (out.is_open()) {
			out.write(val.c_str(), val.size());
		}
	}

}

namespace vm { /* exception_c */

	exception_c::exception_c(msg_t val) 
		: m_msg(val) {
	}

	exception_c::exception_c(const exception_c& val)
		: m_msg(val.get()) {
	}

	exception_c::exception_c(exception_c&& val) noexcept
		: m_msg(std::move(val.get())) {
	}

	exception_c& exception_c::operator=(const exception_c& val) {
		m_msg = val.get();
		return *this;
	}

	exception_c& exception_c::operator=(exception_c&& val) noexcept {
		m_msg = std::move(val.get());
		return *this;
	}

	exception_c::msg_t exception_c::get() const {
		return m_msg;
	}

	exception_c::msg_t& exception_c::get() {
		return m_msg;
	}

}

namespace vm { /* core_c */

	core_c::core_c(const core_c& val) 
		: csx(val.csx)
		, ipx(val.ipx)
		, clx(val.clx) 
		, ssx(val.ssx) 
		, spx(val.spx) 
		, slx(val.slx) 
		, ax(val.ax) 
		, sx(val.sx) 
		, fx(val.fx) {
		for (rega_t idx(0); idx < core_c::xregs; ++idx) {
			x[idx] = val.x[idx];
		}
	}

	core_c::core_c(core_c&& val) noexcept
		: csx(std::move(val.csx))
		, ipx(std::move(val.ipx))
		, clx(std::move(val.clx))
		, ssx(std::move(val.ssx))
		, spx(std::move(val.spx))
		, slx(std::move(val.slx))
		, ax(std::move(val.ax))
		, sx(std::move(val.sx))
		, fx(std::move(val.fx)) {
		for (rega_t idx(0); idx < core_c::xregs; ++idx) {
			x[idx] = std::move(val.x[idx]);
		}
	}

	core_c& core_c::operator=(const core_c& val) {
		for (rega_t idx(0); idx < core_c::xregs; ++idx) {
			x[idx] = val.x[idx];
		}
		csx = val.csx;
		ipx = val.ipx;
		clx = val.clx;
		ssx = val.ssx;
		spx = val.spx;
		slx = val.slx;
		ax = val.ax;
		sx = val.sx;
		fx = val.fx;
		return *this;
	}

	core_c& core_c::operator=(core_c&& val) noexcept {
		for (rega_t idx(0); idx < core_c::xregs; ++idx) {
			x[idx] = std::move(val.x[idx]);
		}
		csx = std::move(val.csx);
		ipx = std::move(val.ipx);
		clx = std::move(val.clx);
		ssx = std::move(val.ssx);
		spx = std::move(val.spx);
		slx = std::move(val.slx);
		ax = std::move(val.ax);
		sx = std::move(val.sx);
		fx = std::move(val.fx);
		return *this;
	}

	core_c::reg32_t& core_c::get(const rega_t val) {
		if (val < core_c::xregs) {
			return x[val];
		}
		switch (val - core_c::xregs) {
		/* code segment */
		case 0:
			return csx;
		case 1:
			return ipx;
		case 2:
			return clx;

		/* stack segment */
		case 3:
			return ssx;
		case 4:
			return spx;
		case 5:
			return slx;

		/* memory address  */
		case 6:
			return ax;

		/* system flags    */
		case 7:
			return sx;

		/* internal flags  */
		case 8:
			return fx;

		default:
			static reg32_t ret = 0;
			return ret;
		}
	}

	core_c core_c::flush() {
		return core_c(std::move(*this));
	}

}

namespace vm { /* memory_c */

	memory_c::memory_c(idx_t val)
		: m_len(val ? val : memory_c::dlen)
		, m_data(nullptr) {
		try {
			m_data = new loc_t[m_len]{ 0 };
		} catch (...) {
			std::cerr << "bad alloc [memory@length: " << m_len << "]";
		}
	}

	memory_c::~memory_c() {
		delete[] m_data;
	}

	memory_c::idx_t memory_c::length() const {
		return m_len;
	}

	memory_c::loc_t& memory_c::get(const idx_t val) {
		static loc_t ret(0);
		try {
			if (val >= m_len) {
				throw exception_c("bad index");
			}
			return m_data[val];
		} catch (const exception_c& exc) {
			std::cerr << exc.get() << std::endl;
			return ret;
		}
	}

}

#include <cmath> // rand
#include <ctime> // time

namespace vm { /* process_c */

	process_c::process_c(info_t val)
		: id(0)
		, info(val)
		, state() {
	}

	process_c::code_t process_c::load(const path_t val) {
		try {
			code_t src(read(val, std::ios_base::in));
			if (src.empty()) {
				throw exception_c("empty source [" + val + "]");
			}

			code_t ret, tmp;

			auto to_dec = [](const char* hex) {
				if (!hex || strlen(hex) != 2) {
					return 0;
				}
				return hex_dig_to_dec_dig(hex[0]) * 0x10 + hex_dig_to_dec_dig(hex[1]);
			};

			for (auto i : src) {
				if ((i >= '0' && i <= '9') || ((i = toupper(i)) >= 'A' && i <= 'F')) {
					tmp.push_back(i);
				}
				if (tmp.size() == 2) {
					ret.push_back(to_dec(tmp.c_str()));
					tmp.clear();
				}
			}

			if (ret.size() % sizeof(instr_t) != 0) {
				throw exception_c("invalid bytecode source [" + val + "]");
			}
			if (ret.empty()) {
				throw exception_c("empty bytecode source [" + val + "]");
			}

			state.clx = ret.size();
			return ret;
		} catch (const exception_c& exc) {
			std::cerr << exc.get() << std::endl;
			return code_t();
		}
	}

	void process_c::start(const id_t mx, const memory_c::idx_t mx_len) {
		id = rand() % 0xFFFFFFFF;
		state.csx = rand() % mx;
		// state.clx = mx_len - mx; // size of code segment
		state.ipx = state.csx;
		info |= (uint8_t)info_e::STARTED;
	}

}

#define PRC_IS_STARTED(prc) ((prc->info & (uint8_t)process_c::info_e::STARTED) != 0)
#define PRC_CLOSE(prc)       \
	do {                     \
		prc->info &= 0xFFFE; \
		delete m_prc;        \
		prc = nullptr;       \
	} while (false);

#include <string> // std::to_string, std::getline...
#include <thread> // std::this_thread

namespace vm {

	vm_c::vm_c(idx_t val)
		: m_memory(val)
		, m_start(std::chrono::system_clock::now())
		, m_prc(nullptr)
		, m_ver(1)
		, m_ec(1) {
		srand(time(nullptr));
	}

	int32_t vm_c::start() {

		std::cin.clear();
		std::cout.clear();
		std::cerr.clear();

		int32_t ret(1);
		time_point beg;
		uint8_t dbg(0);

		static core_c::reg32_t mx_ip(0);

		while (true) {
			if (m_prc && PRC_IS_STARTED(m_prc)) {
				if (m_state.ipx < mx_ip) {
					ret = engine(
						m_memory.get(m_state.ipx),
						m_memory.get(m_state.ipx + 1),
						m_memory.get(m_state.ipx + 2),
						m_memory.get(m_state.ipx + 3)
					);
					m_state.ipx += 4;

					if (dbg) { // view
						ret = view(dbg);
					}
				} else {
					ret = 0;
				}

				// check special codes

				if (ret == 0) { // close
					auto end(std::chrono::system_clock::now());
					std::cout << "process (" << m_prc->id << ") ended with " << m_ec
						<< std::endl << "time elapsed: "
						<< std::chrono::duration_cast<std::chrono::duration<double>>(end - beg).count() << "s"
						<< std::endl;
					PRC_CLOSE(m_prc);
				}
			} else {
				ret = menu();

				// check special codes

				switch (ret) {
				case 1: // close
					beg = std::chrono::system_clock::now();
					m_state = m_prc->state;
					mx_ip = m_state.csx + m_state.clx;
					m_state.ipx = m_state.csx;
					break;

				/* debug */
				case 2:
					dbg = 1; // show registers
					break;
				case 3:
					dbg = 2; // show stack
					break;
				case 4:
					dbg = 3; // show registers and stack
					break;
				case 5:
					dbg = 4; // stop after each instruction and show registers and stack
					break;

				default:
					dbg = 0;
					break;
				}
			}

			if (ret == -1) {
				break;
			}
		}

		return ret;
	}

	int32_t vm_c::engine(const loc_t a, const loc_t b, const loc_t c, const loc_t d) {
		core_c::reg32_t val(0);

		switch (a) {
		case 0x00: // nop
			break;

		case 0x01: // ldx x,v
			m_state.get(b) = (static_cast<core_c::reg32_t>(c) << 8) | d;
			if (b == core_c::xregs + 5) { // slx
				while (true) {
					m_state.ssx = rand() % 0xFFFFFFFF;
					if (
						(m_state.ssx + m_state.slx < m_state.csx) ||
						(m_state.ssx > m_state.csx + m_state.clx)
						) {
						break;
					}
				}
			}
			break;

		case 0x02: // ldx x,x
			m_state.get(b) = m_state.get(c);
			if (b == core_c::xregs + 5) { // slx
				while (true) {
					m_state.ssx = rand() % 0xFFFFFFFF;
					if (
						(m_state.ssx + m_state.slx < m_state.csx) ||
						(m_state.ssx > m_state.csx + m_state.clx)
						) {
						break;
					}
				}
			}
			break;

		case 0x03: // set v
			m_memory.get(m_state.ax) = b;
			break;

		case 0x04: // set x
			m_memory.get(m_state.ax) = m_state.get(b);
			break;

		case 0x05: // get x
			m_state.get(b) = m_memory.get(m_state.ax);
			break;

		case 0x06: // exc v
			return execute(
				(static_cast<core_c::reg32_t>(b) << 0x10) | (static_cast<core_c::reg32_t>(c) << 8) | d
			);

		case 0x07: // exc x
			return execute(m_state.get(b));

		case 0x08: // jit v,v
			if (m_state.fx == d) {
				m_state.ipx = m_state.csx + ((static_cast<core_c::reg32_t>(b) << 8) | c);
			}
			break;

		case 0x09: // jit v,x
			if (m_state.fx == m_state.get(d)) {
				m_state.ipx = m_state.csx + ((static_cast<core_c::reg32_t>(b) << 8) | c);
			}
			break;

		case 0x0A: // jit x,v
			if (m_state.fx == ((static_cast<core_c::reg32_t>(c) << 8) | d)) {
				m_state.ipx = m_state.csx + m_state.get(b);
			}
			break;

		case 0x0B: // jit x,x
			if (m_state.fx == m_state.get(c)) {
				m_state.ipx = m_state.csx + m_state.get(b);
			}
			break;

		case 0x0C: // jif v,v
			if (m_state.fx != d) {
				m_state.ipx = m_state.csx + ((static_cast<core_c::reg32_t>(b) << 8) | c);
			}
			break;

		case 0x0D: // jif v,x
			if (m_state.fx != m_state.get(d)) {
				m_state.ipx = m_state.csx + ((static_cast<core_c::reg32_t>(b) << 8) | c);
			}
			break;

		case 0x0E: // jif x,v
			if (m_state.fx != m_state.get(c)) {
				m_state.ipx = m_state.csx + m_state.get(b);
			}
			break;

		case 0x0F: // jif x,x
			if (m_state.fx != m_state.get(c)) {
				m_state.ipx = m_state.csx + m_state.get(b);
			}
			break;

		case 0x10: // add x,v
			m_state.get(b) += (static_cast<core_c::reg32_t>(c) << 8) | d;
			break;

		case 0x11: // add x,x
			m_state.get(b) += m_state.get(c);
			break;

		case 0x12: // sub x,v
			compare(m_state.get(b), m_state.get(c));
			m_state.get(b) -= (static_cast<core_c::reg32_t>(c) << 8) | d;
			break;

		case 0x13: // sub x,x
			compare(m_state.get(b), m_state.get(c));
			m_state.get(b) -= m_state.get(c);
			break;

		case 0x14: // mul x,v
			m_state.get(b) *= (static_cast<core_c::reg32_t>(c) << 8) | d;
			break;

		case 0x15: // mul x,x
			m_state.get(b) *= m_state.get(c);
			break;

		case 0x16: // div x,v
			val = (static_cast<core_c::reg32_t>(c) << 8) | d;
			if (throw_if(val == 0, "math [0 as divisor]")) {
				return 0;
			}
			m_state.get(b) /= val;
			break;

		case 0x17: // div x,x
			if (throw_if(m_state.get(c) == 0, "math [0 as divisor]")) {
				return 0;
			}
			m_state.get(b) /= m_state.get(c);
			break;

		case 0x18: // and x,v
			m_state.get(b) &= (static_cast<core_c::reg32_t>(c) << 8) | d;
			break;

		case 0x19: // and x,x
			m_state.get(b) &= m_state.get(c);
			break;

		case 0x1A: // or x,v
			m_state.get(b) |= (static_cast<core_c::reg32_t>(c) << 8) | d;
			break;

		case 0x1B: // or x,x
			m_state.get(b) |= m_state.get(c);
			break;

		case 0x1C: // xor x,v
			m_state.get(b) ^= (static_cast<core_c::reg32_t>(c) << 8) | d;
			break;

		case 0x1D: // xor x,x
			m_state.get(b) ^= m_state.get(c);
			break;

		case 0x1E: // shl x,v
			m_state.get(b) <<= (static_cast<core_c::reg32_t>(c) << 8) | d;
			break;

		case 0x1F: // shl x,x
			m_state.get(b) <<= m_state.get(c);
			break;

		case 0x20: // shr x,v
			m_state.get(b) >>= (static_cast<core_c::reg32_t>(c) << 8) | d;
			break;

		case 0x21: // shr x,x
			m_state.get(b) >>= m_state.get(c);
			break;

		case 0x22: // not x
			m_state.get(b) = ~m_state.get(b);
			break;

		case 0x23: // cmp x,v
			compare(m_state.get(b), (static_cast<core_c::reg32_t>(c) << 8) | d);
			break;

		case 0x24: // cmp x,x
			compare(m_state.get(b), m_state.get(c));
			break;

		default:
			throw_if(true, "process (" + std::to_string(m_prc->id) 
				+ ") has an invalid instruction [" + to_hex(a) + " " + to_hex(b) + " "
				+ to_hex(c) + " " + to_hex(d) + "]");
			return 0;
		}

		return 1;
	}

	int32_t vm_c::execute(const uint32_t val) {

		// used by input methods
		static uint8_t vc(0);
		static int32_t vi(0);
		static float32_t vf(0);

		// used by string input and output methods
		uint32_t ptr(m_state.x[0]), len(m_state.x[1]), idx(ptr);
		static std::string str;

		switch (val) {
		case 0x00000001: // exit, abort...

			switch (m_state.sx) {
			case 0x00000001: // exit
				m_ec = to_type<ecode_t, core_c::reg32_t>(m_state.x[0]);
				return 0;

			case 0x00000002: // abort
				throw_if(true, "process (" + std::to_string(m_prc->id) + ") aborted");
				m_ec = -1;
				return 0;

			default:
				break;
			}

			break;

		case 0x00000002: // [console] input, output...

			switch (m_state.sx) {
			case 0x00000001: // [output] char
				std::cout << (char)m_state.x[0];
				break;

			case 0x00000002: // [output] unsigned integer number
				std::cout << m_state.x[0];
				break;

			case 0x00000003: // [output] signed integer number
				std::cout << to_type<core_c::reg32_t, int>(m_state.x[0]);
				break;

			case 0x00000004: // [output] floating point number
				std::cout << to_type<core_c::reg32_t, float>(m_state.x[0]);
				break;

			case 0x00000005: // [output] string
				while (idx < ptr + len) {
					std::cout << m_memory.get(idx);
					++idx;
				}
				break;

			case 0x00000006: // [input] char
				std::getline(std::cin, str);
				m_state.x[0] = str.front();
				break;

			case 0x00000007: // [input] unsigned integer number
				std::cin >> m_state.x[0];
				break;

			case 0x00000008: // [input] signed integer number
				std::cin >> vi;
				m_state.x[0] = to_type<int, core_c::reg32_t>(vi);
				break;

			case 0x00000009: // [input] floating point number
				std::cin >> vf;
				m_state.x[0] = to_type<float, core_c::reg32_t>(vf);
				break;

			case 0x0000000A: // [input] string
				std::getline(std::cin, str);
				m_state.x[0] = str.size();
				for (auto i : str) {
					++m_state.spx;
					m_memory.get(m_state.spx) = i;
				}
				break;

			case 0x0000000B: // [output] clear screen
				system("cls");
				break;

			default:
				break;
			}

			break;

		case 0x00000003: // [file] input, output...

			// coming soon...

			switch (m_state.sx) {
			case 0x00000001: // open file
				break;

			case 0x00000002: // close file
				break;

			case 0x00000003: // remove file
				break;

			default:
				break;
			}

			break;

		default:
			break;
		}
	}

	bool vm_c::throw_if(const bool cnd, const msg_t val) {
		try {
			if (cnd) {
				throw exception_c(val);
			}
		} catch (const exception_c& exc) {
			std::cerr << exc.get() << std::endl;
			return true;
		}
		return false;
	}

	void vm_c::compare(const uint32_t left, const uint32_t right) {
		if (left < right) {
			m_state.fx = 0x0001; // less flag
		} else if (left == right) {
			m_state.fx = 0x0002; // equal flag
		} else {
			m_state.fx = 0x0004; // greater flag
		}
	}

	void vm_c::show_regs() {
		std::cout << "\n" << msg_t(47, '-') << "\n";
		std::cout << "registers" << "\n";
		std::cout << msg_t(47, '-') << "\n";
		core_c::rega_t i(1);
		for (core_c::rega_t idx(0); idx < core_c::xregs; ++idx, ++i) {
			std::cout << "[x" << idx + 1;
			if (idx + 1 < 10) {
				std::cout << " ";
			}
			std::cout << "][" << to_hex(m_state.x[idx]) << "]\t";
			if (i == 3) {
				std::cout << "\n";
				i = 0;
			}
		}
		std::cout << "\n" << msg_t(47, '-') << "\n";
		std::cout << "[csx][" << to_hex(m_state.csx) << "]\t";
		std::cout << "[ipx][" << to_hex(m_state.ipx) << "]\t";
		std::cout << "[clx][" << to_hex(m_state.clx) << "]\t\n";
		std::cout << msg_t(47, '-') << "\n";
		std::cout << "[ssx][" << to_hex(m_state.ssx) << "]\t";
		std::cout << "[spx][" << to_hex(m_state.spx) << "]\t";
		std::cout << "[slx][" << to_hex(m_state.slx) << "]\t\n";
		std::cout << msg_t(47, '-') << "\n";
		std::cout << "[ax][" << to_hex(m_state.ax) << "]\t";
		std::cout << "[sx][" << to_hex(m_state.sx) << "]\t";
		std::cout << "[fx][" << to_hex(m_state.fx) << "]\t\n";
		std::cout << msg_t(47, '-') << "\n";
	}

	void vm_c::show_stack() {
		std::cout << "\n" << msg_t(47, '-') << "\n";
		std::cout << "stack" << "\n";
		std::cout << msg_t(47, '-') << "\n";
		idx_t i(1);
		for (idx_t idx(m_state.ssx); idx < m_state.spx; ++idx, ++i) {
			std::cout << "[" << to_hex(m_state.ssx + idx) << "][" << to_hex(m_memory.get(idx)) << "]\t";
			if (i == 3) {
				std::cout << "\n";
				i = 0;
			}
		}
		if (m_state.ssx == m_state.spx || m_state.slx == 0 || m_state.ssx == 0 || m_state.spx == 0) {
			std::cout << "empty stack...";
		}
		std::cout << "\n" << msg_t(47, '-') << "\n";
	}

	int32_t vm_c::view(const uint8_t val) {
		switch (val) {
		case 1:
			show_regs();
			break;
		case 2:
			show_stack();
			break;
		case 3:
			show_regs();
			show_stack();
			break;
		case 4:
			show_regs();
			show_stack();
			std::cout << "\n" << msg_t(47, '-') << "\n";
			char c;
			std::cout << "press 'b' to break or other key to continue: ";
			std::cin >> c;
			std::cout << msg_t(47, '-') << "\n";
			if ((c = tolower(c)) == 'b') {
				return 0;
			}
			break;
		default:
			break;
		}
		return 1;
	}

#define MAX_HYPENS 50
#define VPAD_CLS 30

	int32_t vm_c::menu() {
		static std::string dir;
		static std::string ver("-----[ QVM " + std::to_string(m_ver) + " ]-----");
		static size_t hypens(MAX_HYPENS - ver.size());

		while (true) {
			bool brk(false);
			char c('\0');
			while (!brk) {
				std::cout << std::string(VPAD_CLS, '\n');

				std::cout << ver << std::string(hypens, '-') << "\n";
				std::cout << "[1] load program\n";
				std::cout << "[2] run program\n";
				std::cout << "[3] debug\n";
				std::cout << "[4] directory\n";
				std::cout << "[0] exit\n";
				std::cout << std::string(MAX_HYPENS, '-') << "\n";
				std::cin.clear();
				std::cin >> c;

				if (c > '4') {
					std::cout << std::string(MAX_HYPENS, '-') << "\n";
					std::cerr << "invalid choice...\n";
					std::cout << std::string(MAX_HYPENS, '-') << "\n";
					std::this_thread::sleep_for(std::chrono::seconds(2));
					brk = false;
				} else {
					brk = true;
				}
			}

			std::string src;

			switch (c) {
			case '0':
				return -1;

			case '1': // load
				brk = false;
				while (!brk) {
					std::cout << std::string(VPAD_CLS, '\n');

					std::cout << ver << std::string(hypens, '-') << "\n";
					std::cout << "program file: ";
					std::cin.clear();
					std::cin >> src;
					std::cout << std::string(MAX_HYPENS, '-') << "\n";

					if (src.empty()) {
						std::cout << std::string(MAX_HYPENS, '-') << "\n";
						std::cerr << "invalid source path...\n";
						std::cout << std::string(MAX_HYPENS, '-') << "\n";
						std::this_thread::sleep_for(std::chrono::seconds(2));
						brk = false;
					} else {
						brk = true;
					}
				}
				m_prc = new process_c();
				m_code = m_prc->load(dir + "/" + src);
				if (m_code.empty()) {
					delete m_prc;
					m_prc = nullptr;
				}
				break;

			case '2': // run
				if (!m_prc) {
					std::cout << std::string(MAX_HYPENS, '-') << "\n";
					std::cerr << "invalid choice...\n";
					std::cout << std::string(MAX_HYPENS, '-') << "\n";
					std::this_thread::sleep_for(std::chrono::seconds(2));
				} else {
					m_prc->start(m_memory.length() - m_code.size(), m_memory.length());
					for (idx_t idx(0); idx < (m_prc->state.clx); ++idx) {
						m_memory.get(m_prc->state.csx + idx) = m_code.at(idx);
					}
					m_code.clear();
					return 1;
				}
				break;

			case '3': // debug
				brk = false;
				while (!brk) {
					std::cout << std::string(VPAD_CLS, '\n');

					std::cout << ver << std::string(hypens, '-') << "\n";
					std::cout << "[1] show registers\n";
					std::cout << "[2] show stack\n";
					std::cout << "[3] show both\n";
					std::cout << "[4] stop after each instruction and show both\n";
					std::cout << "[0] exit\n";
					std::cout << std::string(MAX_HYPENS, '-') << "\n";
					std::cin.clear();
					std::cin >> c;

					if (c > '4') {
						std::cout << std::string(MAX_HYPENS, '-') << "\n";
						std::cerr << "invalid choice...\n";
						std::cout << std::string(MAX_HYPENS, '-') << "\n";
						std::this_thread::sleep_for(std::chrono::seconds(2));
						brk = false;
					} else {
						brk = true;
					}
				}

				if (c == '0') {
					return 0;
				}
				return (c - '0' + 1);

			case '4': // directory
				brk = false;
				while (!brk) {
					std::cout << std::string(VPAD_CLS, '\n');

					std::cout << ver << std::string(hypens, '-') << "\n";
					std::cout << "directory: ";
					std::cin.clear();
					std::cin >> src;
					std::cout << std::string(MAX_HYPENS, '-') << "\n";

					if (src.empty()) {
						std::cout << std::string(MAX_HYPENS, '-') << "\n";
						std::cerr << "invalid source path...\n";
						std::cout << std::string(MAX_HYPENS, '-') << "\n";
						std::this_thread::sleep_for(std::chrono::seconds(2));
						brk = false;
					} else {
						brk = true;
					}
				}
				dir = src;
				break;

			}
		}
		return 0;
	}

}
