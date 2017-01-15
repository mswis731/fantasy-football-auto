#!usr/bin/python

import sys, getopt
from selenium import webdriver
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.ui import WebDriverWait # available since 2.4.0
from selenium.webdriver.support import expected_conditions as EC # available since 2.26.0
from selenium.webdriver.common.by import By
from selenium.webdriver.common.action_chains import ActionChains
import json
from secret import password_trans 
import time

argv = sys.argv[1:]
usage_str = "usage: auto_trans.py [--test] -c <user_config_file> -i <transactions_file> -o <output_file>"

USER_CONFIG_FILE = ''
TRANSACTIONS_FILE = ''
OUTPUT_FILE = ''
TEST_RUN = False
	
try:
	opts, args = getopt.getopt(argv, "hc:i:o:", ["test"])
except getopt.GetoptError:
	print usage_str
	sys.exit(2)
for opt, arg in opts:
	if opt == '-h':
		print usage_str
		sys.exit()
	elif opt == '-c':
		USER_CONFIG_FILE = arg
	elif opt == '-i':
		TRANSACTIONS_FILE = arg
	elif opt == '-o':
		OUTPUT_FILE = arg
	elif opt == '--test':
		TEST_RUN = True

if len(USER_CONFIG_FILE) == 0 or len(TRANSACTIONS_FILE) == 0 or len(OUTPUT_FILE) == 0 or len(args) != 0:
	print usage_str
	sys.exit(2)

# Open/load files
try:
	config = json.load(open(USER_CONFIG_FILE, "r"))
except:
	print "Unable to open input file: {}".format(USER_CONFIG_FILE)
	sys.exit(2)
try:
	trans_info = json.load(open(TRANSACTIONS_FILE, "r"))
except:
	print "Unable to open input file: {}".format(TRANSACTIONS_FILE)
	sys.exit(2)
try:
	output = open(OUTPUT_FILE, "w+");
except:
	print "Unable to open output file: {}".format(OUTPUT_FILE)
	sys.exit(2)

# read user config file
try:
	USERNAME = config["username"]
	PASSWORD = password_trans(config["password"].encode('ascii', errors='ignore'))
except:
	output.write("Invalid user configuration file...exiting\n")
	output.close()
	sys.exit(2)

# read transactions file
try:
	TEAM = trans_info["team"]
	TRANSACTIONS = trans_info["transactions"]
except:
	output.write("Invalid transactions file...exiting\n")
	output.close()
	sys.exit(2)

if len(TRANSACTIONS) == 0:
	output.write("Transactions file empty...exiting\n")
	output.close()
	sys.exit(2)

# driver = webdriver.Firefox()
driver = webdriver.Chrome()
driver.get("http://www.espn.com/fantasy/football/")

# Log in
input_available = False
try:
	driver.find_element_by_link_text("Log In").click()
	WebDriverWait(driver, 10).until(EC.frame_to_be_available_and_switch_to_it((By.XPATH,"//iframe[@name='disneyid-iframe']")))
	WebDriverWait(driver, 10).until(EC.presence_of_all_elements_located((By.XPATH,"(//input)")))
	input1 = driver.find_element_by_xpath("(//input)[1]")
	input1.send_keys(USERNAME);
	input2 = driver.find_element_by_xpath("(//input)[2]")
	input2.send_keys(PASSWORD)
	driver.find_element_by_xpath("//button").click()
	# Check if log in was successful
	input_available = True
	WebDriverWait(driver, 5).until(EC.visibility_of_element_located((By.XPATH, "//div[@class='banner message-error message ng-isolate-scope state-active']")))
	# error message visibile
	sys.exit(2)
except SystemExit:
	output.write("Incorrect log in password...exiting\n")
	output.close()
	driver.close()
	sys.exit(2)
except:
	if not input_available:
		output.write("Unable to input credentials...exiting\n")
		output.close()
		driver.close()
		sys.exit(2)
	else:
		driver.switch_to_default_content()
		output.write("Login successful...\n")

# Close 'Personalize ESPN' pop-up if needed
try:
	pop_up_close_button = WebDriverWait(driver, 10).until(EC.element_to_be_clickable((By.XPATH, "//div[@class='lightbox-container']/article/div[@class='btn-close icon-font-before icon-close-solid-before']")))
	pop_up_close_button.click()
except:
	pass	

# Select fantasy team
while 1:
	try:
		fantasy_teams = WebDriverWait(driver, 10).until(EC.presence_of_all_elements_located((By.XPATH, "//li[@class='teams fantasy']/div/ul/li[@class='team']/a[@itemprop='url']")))
	except:
		output.write("Unable to load fantasy teams...exiting\n")
		output.close()
		driver.close()
		sys.exit(2)
	valid_team = False
	for i in range(len(fantasy_teams)):
		text = fantasy_teams[i].find_element_by_xpath(".//span[@class='link-text']").get_attribute('innerHTML')
		if text == TEAM:
			fantasy_team_href = fantasy_teams[i].get_attribute('href')
			valid_team = True
			break
	if not valid_team:
		output.write("Fantasy team not found...exiting\n")
		output.close()
		driver.close()
		sys.exit(2)
	hover_element = WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, "//li[@class='pillar logo fantasy fantasy']/a")))
	ActionChains(driver).move_to_element(hover_element).perform()
	try:
		WebDriverWait(driver, 10).until(EC.visibility_of_element_located((By.XPATH, "//li[@class='teams fantasy']/div/ul/li[@class='team']/a[@href='{}']".format(fantasy_team_href)))).click()
		break
	except:
		driver.refresh() # try again if hover action didn't work

# Go through transactions
FANTASY_TEAM_HOMEPAGE = driver.current_url
for i in range(len(TRANSACTIONS)):
	log_str = ""

	transaction = TRANSACTIONS[i]
	add_player = transaction["add"]
	drop_player = transaction["drop"]
	# Check if player has been already dropped in previous transaction
	drop_available = False
	try:
		WebDriverWait(driver, 10).until(EC.presence_of_all_elements_located((By.LINK_TEXT, drop_player)))
		log_str += "\tDROP:\t{} FOUND on roster\n".format(drop_player)
		drop_available = True
	except:
		log_str += "\tDROP:\t{} NOT FOUND on roster\n".format(drop_player)

	if drop_available or TEST_RUN:
		# Click Add Player -> All
		hover_element = WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, "//ul[@id='games-subnav-links']/li[@class=' games-subnav-drop-btn']/a")))
		ActionChains(driver).move_to_element(hover_element).perform()
		WebDriverWait(driver, 10).until(EC.visibility_of_element_located((By.LINK_TEXT, "All"))).click()
		# Type in add player into search bar
		WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, "//input[@id='lastNameInput'][@type='text']"))).send_keys(add_player)
		WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, "//input[@class='lastNameFilterButton'][@type='button']"))).click()
		# Check if add_player is still available
		add_available = False
		tries = 0
		MAX_TEST_RUN_TRIES = 2
		while 1:
			try:
				add_player_elem = WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.LINK_TEXT, add_player)))
			except:
				break
			try:
				WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, "//td[@style='text-align: center; white-space: nowrap;']/a")))
				button_elem = add_player_elem.find_element_by_xpath("../..//td[@style='text-align: center; white-space: nowrap;']/a[@title='Add']")
				# button_elem = add_player_elem.find_element_by_xpath("../..//td[@style='text-align: center; white-space: nowrap;']/a[@title='Claim']")
				add_available = True
				break
			except:
				# print "{} not ready to add, still on waivers".format(add_player)
				time.sleep(30) # wait and try again
				tries += 1
				if TEST_RUN and tries >= MAX_TEST_RUN_TRIES:
					break
				driver.refresh()
				continue
		if not add_available:
			log_str += "\tADD:\t{} already claimed or added\n".format(add_player)
		# add_player is available
		else:
			log_str += "\tADD:\t{} is available\n".format(add_player)
			if not TEST_RUN:
				try:
					button_elem.click()
					drop_player_elem = WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.LINK_TEXT, drop_player)))
					checkbox = drop_player_elem.find_element_by_xpath("../..//td[@class='playertableCheckbox']")
					checkbox.click()
					driver.find_element_by_xpath("//input[@value='Submit Roster']").click()
					WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, "//input[@value='Confirm']"))).click()
					log_str += "\tTransaction complete\n"
				except:
					log_str += "\tStale transaction\n"
	# write log to output file
	state = "PASS" if (drop_available and add_available) else "FAIL"
	output.write("[{}]\tTransaction {}:\n{}".format(state, (i+1), log_str))
	
	driver.get(FANTASY_TEAM_HOMEPAGE)
	
driver.close()
output.close()
