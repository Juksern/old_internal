#pragma once
#include "includes.hh"

using proc_t = void(*)();

class vmt_base_hook
{
protected:
	constexpr vmt_base_hook() = default;

public:
  ~vmt_base_hook()
	{
		if(m_new_vmt)
			delete[] (m_new_vmt - 1);
	}

  vmt_base_hook(const vmt_base_hook&) = delete;
  vmt_base_hook(vmt_base_hook&&) = delete;

  vmt_base_hook& operator=(const vmt_base_hook&) = delete;
  vmt_base_hook& operator=(vmt_base_hook&&) = delete;

protected:
	auto initialize(proc_t* original_table) -> void
	{
		m_old_vmt = original_table;

		size_t size = 0;
		while(m_old_vmt[size] && (void*)(m_old_vmt[size]) != nullptr)
			++size;

		m_new_vmt = (new proc_t[size + 1]) + 1;
		//std::copy(m_old_vmt - 1, m_old_vmt + size, m_new_vmt - 1);
		memcpy(m_new_vmt - 1, m_old_vmt - 1, sizeof(void*) * (size + 1));
	}

	constexpr auto leak_table() -> void
	{
		m_new_vmt = nullptr;
	}

	auto hook_instance(void* inst) const -> void
	{
		auto& vtbl = *reinterpret_cast<proc_t**>(inst);
		vtbl = m_new_vmt;
	}

	auto unhook_instance(void* inst) const -> void
	{
		auto& vtbl = *reinterpret_cast<proc_t**>(inst);
		vtbl = m_old_vmt;
	}

	auto initialize_and_hook_instance(void* inst) -> bool
	{
		auto& vtbl = *reinterpret_cast<proc_t**>(inst);
		auto initialized = false;
		if(!m_old_vmt)
		{
			initialized = true;
			initialize(vtbl);
		}
		hook_instance(inst);
		return initialized;
	}

	template <typename Fn>
	auto hook_function(Fn hooked_fn, const size_t index) -> Fn
	{
		m_new_vmt[index] = (proc_t)(hooked_fn);
		return (Fn)(m_old_vmt[index]);
	}

	template<typename T>
	auto apply_hook(size_t idx) -> void
	{
		T::m_original = hook_function(&T::hooked, idx);
	}

	template <typename Fn = uintptr_t>
	auto get_original_function(const int index) -> Fn
	{
		return (Fn)(m_old_vmt[index]);
	}

private:
	proc_t* m_new_vmt = nullptr;
	proc_t* m_old_vmt = nullptr;
};

class VMT : vmt_base_hook {
public:
	VMT(void* class_base)
		: m_class{class_base}
	{
		initialize_and_hook_instance(class_base);
	}

	~VMT()
	{
		unhook_instance(m_class);
	}

  VMT(const VMT&) = delete;
  VMT(VMT&&) = delete;

  VMT& operator=(const VMT&) = delete;
  VMT& operator=(VMT&&) = delete;

	auto rehook() const -> void
	{
		hook_instance(m_class);
	}

	auto unhook() const -> void
	{
		unhook_instance(m_class);
	}

	using vmt_base_hook::apply_hook;
	using vmt_base_hook::get_original_function;
	using vmt_base_hook::hook_function;

private:
	void* m_class = nullptr;
};
