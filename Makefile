BOARD = modwifi

esp.mk:
	curl -sL https://raw.githubusercontent.com/reitermarkus/makeEspArduino/master/makeEspArduino.mk -o esp.mk

-include esp.mk
