from selenium import webdriver
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.ui import WebDriverWait # available since 2.4.0
from selenium.webdriver.support import expected_conditions as EC # available since 2.26.0
from selenium.webdriver.common.by import By
from selenium.webdriver.common.action_chains import ActionChains
import json
from secret import password_trans 
import time

# TODO: logging to file, logging error messages along with personal messages?
# TODO: encrypting passwords and other information

FILE_NAME = "data.json"

data = json.load(open(FILE_NAME, "r"))["players"]

# driver = webdriver.Firefox()
driver = webdriver.Chrome()
driver.get("http://www.espn.com/fantasy/football/")

for player in data:
	USERNAME = player["username"]
	PASSWORD = password_trans(player["password"].encode('ascii', errors='ignore'))
	TEAM_NAME = player["team"]
	TRANSACTIONS = player["transactions"]
	# Log in
	try:
		driver.find_element_by_link_text("Log In").click()
		WebDriverWait(driver, 10).until(EC.frame_to_be_available_and_switch_to_it((By.XPATH,"//iframe[@name='disneyid-iframe']")))
		WebDriverWait(driver, 10).until(EC.presence_of_all_elements_located((By.XPATH,"(//input)")))
		input1 = driver.find_element_by_xpath("(//input)[1]")
		input1.send_keys(USERNAME);
		input2 = driver.find_element_by_xpath("(//input)[2]")
		input2.send_keys(PASSWORD)
		driver.find_element_by_xpath("//button").click()
		driver.switch_to_default_content()
	except:
		print "Login failed"

	# Close 'Personalize ESPN' pop-up if needed
	try:
		pop_up_close_button = WebDriverWait(driver, 10).until(EC.element_to_be_clickable((By.XPATH, "//div[@class='lightbox-container']/article/div[@class='btn-close icon-font-before icon-close-solid-before']")))
		pop_up_close_button.click()
	except:
		None
	
	while 1:
		# Select fantasy team
		fantasy_teams = WebDriverWait(driver, 10).until(EC.presence_of_all_elements_located((By.XPATH, "//li[@class='teams fantasy']/div/ul/li[@class='team']/a[@itemprop='url']")))
		for i in range(len(fantasy_teams)):
			text = fantasy_teams[i].find_element_by_xpath(".//span[@class='link-text']").get_attribute('innerHTML')
			if text == TEAM_NAME:
				fantasy_team_href = fantasy_teams[i].get_attribute('href')
				break
		hover_element = WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, "//li[@class='pillar logo fantasy fantasy']/a")))
		ActionChains(driver).move_to_element(hover_element).perform()
		try:
			WebDriverWait(driver, 10).until(EC.visibility_of_element_located((By.XPATH, "//li[@class='teams fantasy']/div/ul/li[@class='team']/a[@href='{}']".format(fantasy_team_href)))).click()
			break
		except:
			driver.refresh() # try again if hover action didn't work
	
	

	# Go through transcations
	FANTASY_TEAM_HOMEPAGE = driver.current_url
	for i in range(len(TRANSACTIONS)):
		print "Transaction {}:".format(i+1)
		transaction = TRANSACTIONS[i]
		ADD_PLAYER = transaction["add"]
		DROP_PLAYER = transaction["drop"]

		# Check if player has been already dropped in previous transaction
		try:
			WebDriverWait(driver, 10).until(EC.presence_of_all_elements_located((By.LINK_TEXT, DROP_PLAYER)))
			print "{} FOUND on roster".format(DROP_PLAYER)
		except:
			print "{} NOT found on roster".format(DROP_PLAYER)
			continue

		# Click Add Player -> All
		hover_element = WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, "//ul[@id='games-subnav-links']/li[@class=' games-subnav-drop-btn']/a")))
		ActionChains(driver).move_to_element(hover_element).perform()
		WebDriverWait(driver, 10).until(EC.visibility_of_element_located((By.LINK_TEXT, "All"))).click()

		WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, "//input[@id='lastNameInput'][@type='text']"))).send_keys(ADD_PLAYER)
		WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, "//input[@class='lastNameFilterButton'][@type='button']"))).click()

		# Check if ADD_PLAYER is still available
		available = False
		while 1:
			try:
				add_player_elem = WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.LINK_TEXT, ADD_PLAYER)))
			except:
				print "{} already claimed or added".format(ADD_PLAYER)
				break
			try:
				WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, "//td[@style='text-align: center; white-space: nowrap;']/a")))
				button_elem = add_player_elem.find_element_by_xpath("../..//td[@style='text-align: center; white-space: nowrap;']/a[@title='Add']")
				#button_elem = add_player_elem.find_element_by_xpath("../..//td[@style='text-align: center; white-space: nowrap;']/a[@title='Claim']")
				print "{} ready to add".format(ADD_PLAYER)
				available = True
				break
			except:
				print "{} not ready to add, still on waivers".format(ADD_PLAYER)
				time.sleep(30) # wait and try again
				driver.refresh()
				continue

		# ADD_PLAYER is available
		if available:
			button_elem.click()
			drop_player_elem = WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.LINK_TEXT, DROP_PLAYER)))
			checkbox = drop_player_elem.find_element_by_xpath("../..//td[@class='playertableCheckbox']")
			checkbox.click()
			driver.find_element_by_xpath("//input[@value='Submit Roster']").click()
			WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, "//input[@value='Confirm']"))).click()
		driver.get(FANTASY_TEAM_HOMEPAGE)
	driver.close()
