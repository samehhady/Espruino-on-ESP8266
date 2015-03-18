//
//  icache.h
//
//  Created by Blaž Jugovic on 18/03/2015.
//  Copyright (c) 2015 VisioLights. All rights reserved.
//

#ifndef icache_h
#define icache_h

//#define ICACHE_STORE_ATTR __attribute__((aligned(4)))
//#define ICACHE_STORE_TYPEDEF_ATTR __attribute__((aligned(4),packed))
#define ICACHE_RAM_ATTR __attribute__((section(".iram0.text")))

#endif
