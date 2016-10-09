CXXFLAGS := $(patsubst %,-I%,$(INC_DIR)) -std=c++14 -g -fPIC -O0

BIND_ALL := -z now

LD_FLAGS := # $(BIND_ALL) 

%.o : %.cpp
	g++ $(CXXFLAGS) -c $< -o $@

LIB_TARGETS=$(patsubst %,lib%.so,$(LIBRARIES))

$(LIB_TARGETS) : $$($$(patsubst lib%.so,%,$$@)_OBJS)
	g++ $(CXXFLAGS) -shared $^ -o $@ $(LD_FLAGS)
	readelf -Wa $@ | c++filt > $(@:%.so=%.elf)

$(BINARIES) : $$(@).cpp $$(patsubst %,lib%.so,$$($$@_LIBRARIES))
	g++ $(CXXFLAGS) $(@).cpp -o $@ $(LD_FLAGS) $(patsubst %,-L%,$(LIB_DIR)) $(patsubst %,-l%,$($@_LIBRARIES) $($@_LIBS)) -rdynamic -ldl
	readelf -Wa $@ | c++filt > $(@).elf

clean :
	rm -f *.o *.so $(BINARIES) *.elf
