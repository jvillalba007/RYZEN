/*
 * compactator.h
 *
 *  Created on: Jun 26, 2019
 *      Author: axel
 */

#ifndef COMPACTATOR_H_
	#define COMPACTATOR_H_

	int compactate(char*);
	int rename_temporal_files(char*);
	int clean_blocks(char*);
	void recreate_partitions(char*, int, t_list*);
	t_list* get_last_rows(char*);

	// TODO:
	void block_table(char*);
	void unblock_table(char*);
	int kill_compactator_thread(char*);


#endif /* COMPACTATOR_H_ */
