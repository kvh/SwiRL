EXTRA_DIST = configure data model_charniak model_swirl corpus
SUBDIRS = m4 src

#
# These settings are required for training 
# 
MODEL_DIR = SET_ME_FOR_TRAINING
WORD_FILE = corpus/train.words
NE_FILE = corpus/train.ne
CHARNIAK_FILE = corpus/train.synt.cha
PROP_FILE = corpus/train.props
CONVERT_TREEBANK_BIN = src/bin/convert_treebank 
STATS_BIN = src/bin/swirl_corpus_stats
MAKE_SAMPLES_BIN = src/bin/swirl_make_samples
MAKE_BINARY_SAMPLES_BIN = src/bin/swirl_make_binary_samples
AB_LEARNER = src/bin/ab_learner
CONVERTED_FILE = $(MODEL_DIR)/train.converted

#
# These settings are required for testing
#
SWIRL_MODEL_DIR = model_swirl
CHARNIAK_MODEL_DIR = model_charniak
CONVERT_FOR_TEST=src/bin/convert_for_test
SWIRL_BIN = src/bin/swirl_parse_classify
SWIRL_ANALYSIS = src/bin/swirl_corpus_analysis
SCORING_SCRIPT = src/bin/srl-eval.pl

train: train_samples train_binary train_clean

train_samples:
	$(CONVERT_TREEBANK_BIN) $(WORD_FILE) $(NE_FILE) $(CHARNIAK_FILE) $(PROP_FILE) > $(CONVERTED_FILE)
	cp data/verbs.args $(MODEL_DIR)
	cp data/words.temporal $(MODEL_DIR)
	$(STATS_BIN) $(CONVERTED_FILE) $(MODEL_DIR) 
	$(MAKE_SAMPLES_BIN) $(CONVERTED_FILE) $(MODEL_DIR) 
	rm -f $(CONVERTED_FILE)

train_binary:
	for i in A0 A1 A2 A3 A4 A5 AA AM-ADV AM-CAU AM-DIR AM-DIS AM-EXT AM-LOC AM-MNR AM-MOD AM-NEG AM-PNC AM-PRD AM-REC AM-TMP C-A0 C-A1 C-A2 C-A3 C-AM-MNR O R-A0 R-A1 R-A2 R-A3 R-AM-CAU R-AM-LOC R-AM-MNR R-AM-PNC R-AM-TMP; do \
		echo "================================================================="; \
		echo "Generating binary classifier for label $$i..."; \
		echo "================================================================="; \
		echo "NOTE: learning one classifier may take up to 2 hours!"; \
		date; \
		$(MAKE_BINARY_SAMPLES_BIN) $$i $(MODEL_DIR); \
		$(AB_LEARNER) -D 2 -c $(MODEL_DIR)/TRAIN.B-$$i -m $(MODEL_DIR)/B-$$i.model.ab; \
		rm -f $(MODEL_DIR)/TRAIN.B-$$i; \
		date; \
	done

train_clean:
	rm -f $(MODEL_DIR)/negative.samples
	rm -f $(MODEL_DIR)/positive.samples

test:
	mkdir -p testing
	$(CONVERT_FOR_TEST) corpus/$(PREFIX).words corpus/$(PREFIX).ne corpus/$(PREFIX).props > testing/$(PREFIX).input
	date
	$(SWIRL_BIN) $(SWIRL_MODEL_DIR) $(CHARNIAK_MODEL_DIR) testing/$(PREFIX).input > testing/$(PREFIX).output
	date
	perl $(SCORING_SCRIPT) corpus/$(PREFIX).props testing/$(PREFIX).output > testing/$(PREFIX).score
	cat testing/$(PREFIX).score

test_brown:
	make test PREFIX=test.brown
test_wsj:
	make test PREFIX=test.wsj
test_dev:
	make test PREFIX=devel.24

#
# Make sure FILE is in the format specified in the README
# Take a look at testing/test.wsj.input for an example (after make test_wsj)
#
test_file:
	$(SWIRL_BIN) $(SWIRL_MODEL_DIR) $(CHARNIAK_MODEL_DIR) $(FILE)

#
# This label was used for development (gathering stats etc)
# Not really useful for regular usage
#
analysis:
	$(CONVERT_TREEBANK_BIN) $(WORD_FILE) $(NE_FILE) $(CHARNIAK_FILE) $(PROP_FILE) > /tmp/analysis.converted
	$(SWIRL_ANALYSIS) /tmp/analysis.converted >& /tmp/analysis.log
	rm -f /tmp/analysis.converted
	echo "Analysis log file saved in: /tmp/analysis.log"

bin:
	cd src/bin; make clean 
	make
