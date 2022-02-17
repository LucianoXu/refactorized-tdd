#pragma once
#include "stdafx.h"
#include "weight.h"
#include "succ_ls.hpp"

namespace node {
	template <class W>
	class Node;
	template <class W>
	struct weightednode;
}


namespace cache {
	// the type for unique table
	template <class W>
	struct unique_table_key {
		int order;
		// real for <weight>, data for <tensor> (encode  in sequence)
		std::vector<int> code1;
		// imag for <weight>, shape for <tensor>
		std::vector<int> code2;
		std::vector<const node::Node<W>*> nodes;

		/// <summary>
		/// Construction of a unique_table key (complex version)
		/// </summary>
		/// <param name="_order"></param>
		/// <param name="_range"></param>
		/// <param name="_p_weights">[borrowed]</param>
		/// <param name="_p_nodes">[borrowed</param>
		unique_table_key(int _order, const node::succ_ls<W>& successors);

		unique_table_key(const unique_table_key& other) {
			order = other.order;
			code1 = std::vector<int>(other.code1);
			code2 = std::vector<int>(other.code2);
			nodes = std::vector<const node::Node<W>*>(other.nodes);
		}

		unique_table_key& operator =(unique_table_key&& other) {
			order = other.order;
			code1 = std::move(other.code1);
			code2 = std::move(other.code2);
			nodes = std::move(other.nodes);
			return *this;
		}
	};

	template <class W>
	inline bool operator == (const unique_table_key<W>& a, const unique_table_key<W>& b) {
		if (a.order != b.order) {
			return false;
		}
		// compare code2 first, because for <tensor> case it stores the shape information
		if (a.code2 != b.code2) {
			return false;
		}
		if (a.code1 != b.code1) {
			return false;
		}
		return true;
	}

	template <class W>
	inline std::size_t hash_value(const unique_table_key<W>& key) {
		std::size_t seed = 0;
		boost::hash_combine(seed, key.order);
		for (auto i = key.code1.begin(); i != key.code1.end(); i++) {
			boost::hash_combine(seed, *i);
		}
		for (auto i = key.code2.begin(); i != key.code2.end(); i++) {
			boost::hash_combine(seed, *i);
		}
		for (auto i = key.nodes.begin(); i != key.nodes.end(); i++) {
			boost::hash_combine(seed, *i);
		}
		return seed;
	}

	template <typename W>
	using unique_table = boost::unordered_map<unique_table_key<W>, const node::Node<W>*>;

	// the type for duplicate cache
	template <typename W>
	using duplicate_table = boost::unordered_map<int, const node::Node<W>*>;


	template <class W>
	struct append_table_key {
		int id_a;
		int id_b;
		append_table_key(int _id_a, int _id_b) {
			id_a = _id_a;
			id_b = _id_b;
		}
	};

	// the type for append cache
	template <typename W>
	using append_table = boost::unordered_map<append_table_key<W>, const node::Node<W>*>;




	// the type for CUDAcpl cache
	template <class W>
	using CUDAcpl_table = boost::unordered_map<int, CUDAcpl::Tensor>;


	// the type for summation cache
	template <class  W>
	struct sum_key {
		int id_1;
		int nweight1_real;
		int nweight1_imag;
		std::vector<int> nweight1_real_tensor;
		std::vector<int> nweight1_imag_tensor;
		int id_2;
		int nweight2_real;
		int nweight2_imag;
		std::vector<int> nweight2_real_tensor;
		std::vector<int> nweight2_imag_tensor;

		/// <summary>
		/// Construct the key. Note that id_1 will be set as the smaller one.
		/// </summary>
		/// <param name="id_a"></param>
		/// <param name="weight_a"></param>
		/// <param name="id_b"></param>
		/// <param name="weight_b"></param>
		sum_key(int id_a, const W& weight_a, int id_b, const W& weight_b);
		
		sum_key& operator =(sum_key&& other) {
			id_1 = other.id_1;
			nweight1_real = other.nweight1_real;
			nweight1_imag = other.nweight1_imag;
			nweight1_real_tensor = std::move(other.nweight1_real_tensor);
			nweight1_imag_tensor = std::move(other.nweight1_imag_tensor);
			id_2 = other.id_2;
			nweight2_real = other.nweight2_real;
			nweight2_imag = other.nweight2_imag;
			nweight2_real_tensor = std::move(other.nweight2_real_tensor);
			nweight2_imag_tensor = std::move(other.nweight2_imag_tensor);
			return *this;
		}
	};

	bool operator == (const sum_key<wcomplex>& a, const sum_key<wcomplex>& b);

	std::size_t hash_value(const sum_key<wcomplex>& key_struct);

	template <class W>
	using sum_table = boost::unordered_map<sum_key<W>, node::weightednode<W>>;


	typedef std::vector<std::pair<int, int>> cont_cmd;
	// the type for contraction cache
	template <class W>
	struct cont_key {
		int id;
		// first: the smaller index to trace, second: the larger index to trace
		cont_cmd remained_ls;
		// first: the larger index to trace, seconde; the index value to select
		cont_cmd waiting_ls;

		/// <summary>
		/// Note: all pointer ownership borrowed.
		/// </summary>
		/// <param name="_id"></param>
		/// <param name="_num_remained"></param>
		/// <param name="_p_r_i1"></param>
		/// <param name="_p_r_i2"></param>
		/// <param name="_num_waiting"></param>
		/// <param name="_p_w_i"></param>
		/// <param name="_p_w_v"></param>
		inline cont_key(int _id, const cont_cmd& _remained_ls, const cont_cmd& _waiting_ls) {
			id = _id;
			remained_ls = cont_cmd(_remained_ls);
			waiting_ls = cont_cmd(_waiting_ls);
		}

		inline cont_key(int _id, const cont_cmd& _remained_ls, cont_cmd&& _waiting_ls) {
			id = _id;
			remained_ls = std::move(_remained_ls);
			waiting_ls = std::move(_waiting_ls);
		}

		inline cont_key(const cont_key& other) {
			id = other.id;
			remained_ls = cont_key(other.remained_ls);
			waiting_ls = cont_cmd(other.waiting_ls);
		}
		inline cont_key& operator =(cont_key&& other) {
			id = other.id;
			remained_ls = std::move(other.remained_ls);
			waiting_ls = std::move(other.waiting_ls);
			return *this;
		}
	};

	template <class W>
	inline bool operator == (const cont_key<W>& a, const cont_key<W>& b) {
		return (a.id == b.id && a.remained_ls == b.remained_ls && a.waiting_ls == b.waiting_ls);
	}

	template <class W>
	inline std::size_t hash_value(const cont_key<W>& key) {
		std::size_t seed = 0;
		boost::hash_combine(seed, key.id);
		for (auto i = key.remained_ls.begin(); i != key.remained_ls.end(); i++) {
			boost::hash_combine(seed, i->first);
			boost::hash_combine(seed, i->second);
		}
		for (auto i = key.waiting_ls.begin(); i != key.waiting_ls.end(); i++) {
			boost::hash_combine(seed, i->first);
			boost::hash_combine(seed, i->second);
		}
		return seed;
	}

	template <class W>
	using cont_table = boost::unordered_map<cont_key<W>, node::weightednode<W>>;


	template <class W>
	struct Global_Cache {
		static duplicate_table<W>* p_duplicate_cache;
		static duplicate_table<W>* p_shift_cache;
		static append_table<W>* p_append_cache;
		static CUDAcpl_table<W>* p_CUDAcpl_cache;
		static sum_table<W>* p_sum_cache;
		static cont_table<W>* p_cont_cache;
	};
}