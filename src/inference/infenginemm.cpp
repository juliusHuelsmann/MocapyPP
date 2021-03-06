/*
 * infenginemm.cpp
 *
 *  Copyright (C) 2009, Jes Frellsen, The Bioinformatics Centre, University of Copenhagen.
 *
 *  This file is part of Mocapy++.
 *
 *  Mocapy++ is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mocapy++ is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Mocapy++.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "infenginemm.h"
#include "../inference/detail/generalinfenginemm.h"

#include <assert.h>

#include "../utils/utils.h"
#include "../utils/mdarray.h"


namespace mocapy {

/**********************
 * SampleInfEngineMM *
 **********************/
SampleInfEngineMM::SampleInfEngineMM(DBN * new_dbn, Sequence & new_seq, MDArray<eMISMASK> & new_mismask,
			uint new_hidden_node, bool check_dbn, double new_weight) :
  SampleInfEngine(new_dbn, new_seq, new_mismask), generalinfengine(new_dbn, new_hidden_node, check_dbn) {

  randomGen = new_dbn->randomGen;
  assert(randomGen != NULL);
}

Sequence & SampleInfEngineMM::sample_next() {
	store_seq(); 	// Store the old sequence for possible undo
	set_parentmap(parentmap_list);
	generalinfengine.sample(start, end, seq_len, mismask, randomGen);
	return seq;
}

double SampleInfEngineMM::calc_ll(MDArray<eMISMASK> & new_mismask, int new_start, int new_end, bool multiply_by_input) {
	// Check the new mismask
	check_seq_and_mismask(seq, new_mismask);

	// Set and check start and end
	uint start = new_start;
	uint end = new_end<0 ? seq_len : new_end;

	if (!(0 <= new_start && start < seq_len)) throw MocapyExceptions("SampleInfEngineMM: start must be between in [0, sequence length)");
	if (!(start < end && end <= seq_len))     throw MocapyExceptions("SampleInfEngineMM: end must be between in (start, sequence length]");

	// Put the data in the dbn and calculate the ll of this
	set_parentmap(parentmap_list);
	double ll = generalinfengine.calc_ll(start, end, seq_len, new_mismask, multiply_by_input);

	return ll;
}

/**************************
 * LikelihoodInfEngineMM *
 **************************/
LikelihoodInfEngineMM::LikelihoodInfEngineMM(DBN * new_dbn, uint new_hidden_node, bool check_dbn) :
	MoreAbstractInfEngine(new_dbn), generalinfengine(new_dbn, new_hidden_node, check_dbn) {}

double LikelihoodInfEngineMM::calc_ll(Sequence & seq, MDArray<eMISMASK> & mismask, int new_start, int new_end, bool multiply_by_input) {
	// Check the sequence and mismask
	check_seq_and_mismask(seq, mismask);

	uint seq_len = seq.get_shape()[0];

	// Set and check start and end
	uint start = new_start;
	uint end = new_end<0 ? seq_len : new_end;

	if (!(0 <= new_start && start < seq_len)) throw MocapyExceptions("SampleInfEngineMM: start must be between in [0, sequence length)");
	if (!(start < end && end <= seq_len))     throw MocapyExceptions("SampleInfEngineMM: end must be between in (start, sequence length]");

	// Put the data in the dbn
	set_parentmap(&seq, 1.0);

	// Calculate the likelihood of the current data in the dbn
	double ll = generalinfengine.calc_ll(start, end, seq_len, mismask, multiply_by_input);

	return ll;
}


double LikelihoodInfEngineMM::calc_ll(std::vector<Sequence *> &seqs, std::vector<MDArray<eMISMASK> > &mismasks, bool multiply_by_input) {
	double ll = 0.0;
	for (uint i=0; i<seqs.size(); i++) {
		ll += calc_ll(*seqs[i], mismasks[i], 0, -1, multiply_by_input);
	}
	return ll;
}

}
