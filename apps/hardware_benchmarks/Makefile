clean_all cleanall:
	$(MAKE) -C tests cleanall
	$(MAKE) -C apps cleanall
	$(MAKE) -C hw_support clean

check:
	@echo "*****TESTS*****"
	@$(MAKE) -sC tests check
	@echo ""
	@echo "*****APPS******"
	@$(MAKE) -sC apps check
