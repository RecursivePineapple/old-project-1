
src/Common/ErrorCodes.hpp src/Common/ErrorCodes.cpp: misc/update_errors.py ../error_codes.yml
	python3 misc/update_errors.py


CFG_SRCS = $(shell grep -l -E "^\#pragma configurable" $(C_SRCS))

src/Impl/Configure/ConfigureImpl.cpp: $(CFG_SRCS)
	misc/update_configurables.sh $@ $^
