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
#ifndef REG_TEST_H
#define REG_TEST_H

void test_chip_peripherals(void);
void test_sysram(void);
void test_sdram(void); 	// BUGZILLA 56. CP 20/08/2010
void test_processor_peripherals(void);

#endif
