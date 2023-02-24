/*
 * Copyright (c) 2008 The University of Manchester
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAMTEST_H
	#define RAMTEST_H

	unsigned int ram_test_c (unsigned int startaddr, unsigned int endaddr, unsigned int mask);
	unsigned int ram_test (unsigned int startaddr, unsigned int endaddr, unsigned int mask);

#endif
