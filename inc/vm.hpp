#ifndef Q_INC_VM
#define Q_INC_VM

#include <cstdint>
#include <xstring>
#include <chrono>

namespace vm {

	class exception_c {
	public:

		using msg_t = std::string;

	protected:

		msg_t m_msg;

	public:

		explicit exception_c(msg_t);
		exception_c() = delete;
		exception_c(const exception_c&);
		exception_c(exception_c&&) noexcept;

		exception_c& operator=(const exception_c&);
		exception_c& operator=(exception_c&&) noexcept;

	public:

		~exception_c() = default;

	public:

		msg_t get() const;

	public:

		msg_t& get();

	};

	class core_c {
	public:

		using reg8_t  = uint8_t;
		using reg16_t = uint16_t;
		using reg32_t = uint32_t;
		using reg64_t = uint64_t;

		// register address
		using rega_t  = uint8_t;

	public:

		// numbers of x registers
		static constexpr rega_t xregs = 0x10;

	public:

		reg32_t
			/* general purpose */ x[core_c::xregs]{ 0 },
			/* code segment    */ csx{ 0 }, ipx{ 0 }, /* length */ clx{ 0 },
			/* stack segment   */ ssx{ 0 }, spx{ 0 }, /* length */ slx{ 0 },
			/* memory address  */ ax{ 0 },
			/* system flags    */ sx{ 0 },
			/* internal flags  */ fx{ 0 }
			;

	public:

		core_c() = default;
		core_c(const core_c&);
		core_c(core_c&&) noexcept;

		core_c& operator=(const core_c&);
		core_c& operator=(core_c&&) noexcept;

	public:

		~core_c() = default;

	public:

		// @why: to use a number as register address.
		// @in: address of the register.
		// @out: reference to the register at this address.
		reg32_t& get(const rega_t);

	public:

		// @why: to clear current data.
		// @in: null.
		// @out: the last version of core.
		core_c flush();

	};

	class memory_c {
	public:

		// index type
		using idx_t = uint32_t;

		// block type
		using loc_t = uint8_t;

	public:

		// default number of blocks (128 Mb)
		static constexpr idx_t dlen = 0x08000000;

	protected:

		loc_t* m_data;
		idx_t m_len;

	public:

		explicit memory_c(idx_t = 0);
		memory_c(const memory_c&) = delete;
		memory_c(memory_c&&) noexcept = delete;

		memory_c& operator=(const memory_c&) = delete;
		memory_c& operator=(memory_c&&) noexcept = delete;

	public:

		~memory_c();

	public:

		idx_t length() const;

	public:

		// @why: to access at the protected data.
		// @in: address of a memory location.
		// @out: reference to the location.
		loc_t& get(const idx_t);

	};

	class process_c {
	public:

		using instr_t = uint32_t;

		using id_t = uint32_t;

		// informations about the current state of process
		using info_t = uint16_t;

		enum class info_e : uint16_t {
			STARTED = 0x0001, // setted when start method is called
			ABORTED = 0x0002, // setted when an exception threw
		};

		using path_t = std::string;
		using code_t = std::string;

	public:

		id_t id;
		info_t info;
		core_c state;

	public:

		explicit process_c(info_t = 0);
		process_c(const process_c&) = delete;
		process_c(process_c&&) noexcept = delete;

		process_c& operator=(const process_c&) = delete;
		process_c& operator=(process_c&&) noexcept = delete;

	public:

		~process_c() = default;

	public:

		code_t load(const path_t);

		void start(const id_t, const memory_c::idx_t);

	};

	class vm_c {
	public:

		using time_point = std::chrono::system_clock::time_point;
		using msg_t      = exception_c::msg_t;
		using loc_t      = memory_c::loc_t;
		using idx_t      = memory_c::idx_t;
		using code_t     = process_c::code_t;
		using id_t       = process_c::id_t;
		using instr_t    = process_c::instr_t;

		using version_t  = uint32_t;
		using ecode_t    = int32_t;

	protected:

		version_t m_ver;

		core_c m_state;
		memory_c m_memory;
		time_point m_start;

		process_c* m_prc;
		code_t m_code;

		ecode_t m_ec; // exit code

	public:

		explicit vm_c(idx_t = 0);
		vm_c(const vm_c&) = delete;
		vm_c(vm_c&&) noexcept = delete;

		vm_c& operator=(const vm_c&) = delete;
		vm_c& operator=(vm_c&&) noexcept = delete;

	public:

		~vm_c() = default;

	public:

		int32_t start();

	protected: // engine

		int32_t engine(const loc_t, const loc_t, const loc_t, const loc_t);
		int32_t execute(const uint32_t);
		bool throw_if(const bool, const msg_t);

		void compare(const uint32_t, const uint32_t);

	protected: // user interface

		void show_regs();
		void show_stack();
		int32_t view(const uint8_t);
		int32_t menu();

	};

}

#endif

