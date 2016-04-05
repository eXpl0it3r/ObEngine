//Author : Sygmei
//Key : 976938ef7d46c286a2027d73f3a99467bcfa8ff0c1e10bd0016139744ef5404f4eb4d069709f9831f6de74a094944bf0f1c5bf89109e9855290336a66420376f

#include "Triggers.hpp"

TriggerDatabase triggerDatabaseCore;

//Trigger
Trigger::Trigger(std::string group, std::string triggerName, bool startState, bool permanent)
{
	this->triggerNamespace = "";
	this->group = group;
	this->triggerName = triggerName;
	this->permanent = permanent;
	this->enabled = startState;
	this->triggerParameters = std::map<std::string, std::pair<std::string, emorph::any*>>();
}
Trigger::Trigger(std::string nsp, std::string group, std::string triggerName, bool startState, bool permanent)
{
	this->triggerNamespace = nsp;
	this->group = group;
	this->triggerName = triggerName;
	this->permanent = permanent;
	this->enabled = startState;
	this->triggerParameters = std::map<std::string, std::pair<std::string, emorph::any*>>();
}
bool Trigger::getState()
{
	return enabled;
}
bool Trigger::isPermanent()
{
	return permanent;
}
std::string Trigger::getGroup()
{
	return this->group;
}
std::string Trigger::getName()
{
	return this->triggerName;
}
std::string Trigger::getNamespace()
{
	return triggerNamespace;
}
std::map<std::string, std::pair<std::string, emorph::any*>> Trigger::getParameters()
{
	return triggerParameters;
}
void Trigger::clearParameters()
{
	for (auto i = triggerParameters.begin(); i != triggerParameters.end(); i++)
	{
		delete triggerParameters[i->first].second;
	}
	triggerParameters.clear();
}


//TriggerDelay
TriggerDelay::TriggerDelay(Trigger* trg, int del, bool sta)
{
	trigger = trg;
	delay = del;
	state = sta;
}

//TriggerGroup
TriggerGroup::TriggerGroup(std::string triggerGroupName)
{
	this->fromNsp = "";
	this->triggerGroupName = triggerGroupName;
}
TriggerGroup::TriggerGroup(std::string triggerGroupNamespace, std::string triggerGroupName)
{
	this->fromNsp = triggerGroupNamespace;
	this->triggerGroupName = triggerGroupName;
}
Trigger* TriggerGroup::getTrigger(std::string triggerName)
{
	if (this->triggerMap.find(triggerName) != this->triggerMap.end())
	{
		return this->triggerMap[triggerName];
	}
	else
	{
		std::cout << "<Error:Triggers:TriggerGroup>[getTrigger] : Trigger " << triggerName << " does not exists in group " << triggerGroupName << std::endl;
		return nullptr;
	}
}
TriggerGroup* TriggerGroup::addTrigger(std::string triggerName)
{
	this->triggerMap[triggerName] = new Trigger(this->fromNsp, triggerGroupName, triggerName);
	return this;
}
void TriggerGroup::delayTriggerState(std::string triggerName, int delay, bool state)
{
	this->delayedTriggers.push_back(new TriggerDelay(getTrigger(triggerName), delay, state));
}
void TriggerGroup::enableTrigger(std::string triggerName)
{
	this->getTrigger(triggerName)->toEnable = true;
	if (this->getTrigger(triggerName)->toDisable) this->getTrigger(triggerName)->toDisable = false;
}
void TriggerGroup::disableTrigger(std::string triggerName)
{
	this->getTrigger(triggerName)->toDisable = true;
	if (this->getTrigger(triggerName)->toEnable) this->getTrigger(triggerName)->toEnable = false;
}
void TriggerGroup::setTriggerState(std::string triggerName, bool state)
{
	if (state) this->enableTrigger(triggerName);
	else this->disableTrigger(triggerName);
}
bool TriggerGroup::getState(std::string triggerName)
{
	return this->getTrigger(triggerName)->getState();
}
void TriggerGroup::setPermanent(std::string triggerName, bool permanent)
{
	this->getTrigger(triggerName)->permanent = permanent;
}
std::vector<std::string> TriggerGroup::getAllTriggersName()
{
	std::vector<std::string> returnVec;
	for (auto it = triggerMap.begin(); it != triggerMap.end(); it++)
	{
		returnVec.push_back(it->first);
	}
	return returnVec;
}
std::vector<Trigger*> TriggerGroup::getAllTriggers()
{
	std::vector<Trigger*> returnVec;
	for (auto it = triggerMap.begin(); it != triggerMap.end(); it++)
	{
		returnVec.push_back(it->second);
	}
	return returnVec;
}
std::string TriggerGroup::getNamespace()
{
	return fromNsp;
}

//TriggerDatabase
TriggerDatabase::TriggerDatabase()
{
	coreTriggers["Start"] = new Trigger("Global", "Start");
	databaseChrono.start();
}
Trigger* TriggerDatabase::getTrigger(std::string triggerName)
{
	if (this->coreTriggers.find(triggerName) != this->coreTriggers.end())
	{
		return this->coreTriggers[triggerName];
	}
	else
	{
		std::cout << "<Error:Triggers:TriggerDatabase>[getTrigger] : Core Trigger : " << triggerName << " does not exists" << std::endl;
		return nullptr;
	}
}
Trigger* TriggerDatabase::getCustomTrigger(std::string groupNamespace, std::string triggerGroupName, std::string triggerName)
{
	if (customTriggers.find(groupNamespace) != customTriggers.end())
	{
		if (customTriggers[groupNamespace].find(triggerGroupName) != customTriggers[groupNamespace].end())
		{
			return customTriggers[groupNamespace][triggerGroupName]->getTrigger(triggerName);
		}
		else
		{
			std::cout << "<Error:Triggers:TriggerDatabase>[getCustomTrigger] : Custom Trigger Group : " << triggerGroupName << " does not exists in Group Namespace : " << groupNamespace << std::endl;
		}
	}
	else
	{
		std::cout << "<Error:Triggers:TriggerDatabase>[getCustomTrigger] : Custom Group Namespace : " << groupNamespace << " does not exists" << std::endl;
	}
	return nullptr;
}

void TriggerDatabase::setTriggerState(std::string name, bool state)
{
	if (state) this->getTrigger(name)->toEnable = true;
	else this->getTrigger(name)->toDisable = true;
}

void TriggerDatabase::createCustomNamespace(std::string groupNamespace)
{
	if (customTriggers.size() == 0)
	{
		customTriggers[groupNamespace] = std::map<std::string, TriggerGroup*>();
	}
	else
	{
		if (customTriggers.find(groupNamespace) == customTriggers.end())
		{
			customTriggers[groupNamespace] = std::map<std::string, TriggerGroup*>();
		}
		else
		{
			std::cout << "<Error:Triggers:TriggerDatabase>[createCustomNamespace] : Custom Group Namespace : " << groupNamespace << " already exists" << std::endl;
		}
	}
}

TriggerGroup* TriggerDatabase::createTriggerGroup(std::string groupNamespace, std::string triggerGroupName)
{
	std::cout << "Trying to create TriggerGroup : " << triggerGroupName << " inside : " << groupNamespace << std::endl;
	if (customTriggers.find(groupNamespace) != customTriggers.end())
	{
		if (customTriggers[groupNamespace].find(triggerGroupName) == customTriggers[groupNamespace].end())
		{
			std::cout << "Success !" << std::endl;
			customTriggers[groupNamespace][triggerGroupName] = new TriggerGroup(groupNamespace, triggerGroupName);
			return customTriggers[groupNamespace][triggerGroupName];
		}
		else
		{
			std::cout << "<Error:Triggers:TriggerDatabase>[createTriggerGroup] : Custom Trigger Group : " << triggerGroupName << " already exists in Group Namespace : " << groupNamespace << std::endl;
		}
	}
	else
	{
		std::cout << "<Error:Triggers:TriggerDatabase>[createTriggerGroup] : Custom Group Namespace : " << groupNamespace << " does not exists" << std::endl;
	}
	return nullptr;
}
TriggerGroup* TriggerDatabase::joinTriggerGroup(std::string groupNamespace, std::string triggerGroupName)
{
	std::cout << "Trying to join TriggerGroup : " << triggerGroupName << " inside : " << groupNamespace << std::endl;
	if (customTriggers.find(groupNamespace) != customTriggers.end())
	{
		if (customTriggers[groupNamespace].find(triggerGroupName) == customTriggers[groupNamespace].end())
		{
			std::cout << "<Error:Triggers:TriggerDatabase>[joinTriggerGroup] : Custom Trigger Group : " << triggerGroupName << " does not exists in Group Namespace : " << groupNamespace << std::endl;
		}
		else
		{
			return customTriggers[groupNamespace][triggerGroupName];
		}
	}
	else
	{
		std::cout << "<Error:Triggers:TriggerDatabase>[joinTriggerGroup] : Custom Group Namespace : " << groupNamespace << " does not exists" << std::endl;
	}
	return nullptr;
}
bool TriggerDatabase::doesTriggerGroupExists(std::string groupNamespace, std::string triggerGroupName)
{
	if (customTriggers[groupNamespace].find(triggerGroupName) == customTriggers[groupNamespace].end())
		return false;
	else
		return true;
}
std::vector<std::string> TriggerDatabase::getAllTriggersNameFromCustomGroup(std::string groupNamespace, std::string triggerGroupName)
{
	if (customTriggers.find(groupNamespace) != customTriggers.end())
	{
		if (customTriggers[groupNamespace].find(triggerGroupName) != customTriggers[groupNamespace].end())
		{
			return customTriggers[groupNamespace][triggerGroupName]->getAllTriggersName();
		}
		else
		{
			std::cout << "<Error:Triggers:TriggerDatabase>[getAllTriggersNameFromCustomGroup] : Custom Trigger Group : " << triggerGroupName << " does not exists in Group Namespace : " << groupNamespace << std::endl;
		}
	}
	else
	{
		std::cout << "<Error:Triggers:TriggerDatabase>[getAllTriggersNameFromCustomGroup] : Custom Group Namespace : " << groupNamespace << " does not exists" << std::endl;
	}
	return std::vector<std::string>();
}
void TriggerDatabase::update()
{
	for (auto it = coreTriggers.begin(); it != coreTriggers.end(); it++)
	{
		if (!it->second->isPermanent()) it->second->enabled = false;
		if (it->second->toDisable)
		{
			it->second->enabled = false;
			it->second->toDisable = false;
		}
		if (it->second->toEnable) 
		{
			it->second->enabled = true; 
			it->second->toEnable = false;
		}
	}
	for (auto it = customTriggers.begin(); it != customTriggers.end(); it++)
	{
		for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			if (it2->second->delayedTriggers.size() != 0)
			{
				for (int i = 0; i < it2->second->delayedTriggers.size(); i++)
				{
					it2->second->delayedTriggers[i]->delaytarget = databaseChrono.getTime() + it2->second->delayedTriggers[i]->delay;
					delayedTriggers.push_back(it2->second->delayedTriggers[i]);
				}
				it2->second->delayedTriggers.clear();
			}
			std::vector<Trigger*> trgList = it2->second->getAllTriggers();
			for (int i = 0; i < trgList.size(); i++)
			{
				if (!trgList[i]->isPermanent()) trgList[i]->enabled = false;
				if (trgList[i]->toDisable)
				{
					trgList[i]->enabled = false;
					trgList[i]->toDisable = false;
				}
				if (trgList[i]->toEnable)
				{
					trgList[i]->enabled = true;
					trgList[i]->toEnable = false;
				}
			}
		}
	}
	std::vector<int> triggeredDelayedTriggers;
	for (int i = 0; i < delayedTriggers.size(); i++)
	{
		if (delayedTriggers[i]->delaytarget <= databaseChrono.getTime())
		{
			if (delayedTriggers[i]->state) delayedTriggers[i]->trigger->toEnable = true;
			else delayedTriggers[i]->trigger->toDisable = true;
			delete(delayedTriggers[i]);
			triggeredDelayedTriggers.push_back(i);
		}
	}
	for (int i = 0; i < triggeredDelayedTriggers.size(); i++)
	{
		delayedTriggers.erase(delayedTriggers.begin() + triggeredDelayedTriggers[i]);
	}
}
