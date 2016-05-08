//Author : Sygmei
//Key : 976938ef7d46c286a2027d73f3a99467bcfa8ff0c1e10bd0016139744ef5404f4eb4d069709f9831f6de74a094944bf0f1c5bf89109e9855290336a66420376f

#include "Dialog.hpp"

sf::RenderTexture renTex;

TextRenderer::TextRenderer()
{
	currentRenderer = nullptr;
	renTex.create(1920, 1080);
}

void TextRenderer::createRenderer(std::string rendererType, std::string id)
{
	Renderer* returnRenderer = nullptr;
	if (rendererType == "VisualNovel")
		returnRenderer = new Renderers::VisualNovel;
	else if (rendererType == "Shade")
		returnRenderer = new Renderers::Shade;
	else
		std::cout << "<Error:Dialog:TextRenderer>[createRenderer] : Failed to create Renderer of type : " << rendererType << std::endl;
	returnRenderer->load();
	rendererDB[id] = returnRenderer;
}

void TextRenderer::sendToRenderer(std::string id, std::map<std::string, std::string> tdb)
{
	if (rendererDB.find(id) != rendererDB.end())
	{
		rendererDB[id]->addTDB(tdb);
		rendererCalls.push_back(id);
		if (currentRenderer == nullptr || !this->textRemaining())
		{
			needToRender = true;
			currentRenderer = rendererDB[id];
		}
	}
	else
		std::cout << "<Error:Dialog:TextRenderer>[sendToRenderer] : Can't find Renderer with id : " << id << std::endl;
}

bool TextRenderer::textRemaining()
{
	return rendererCalls.size() > 0;
}

void TextRenderer::next()
{
	if (rendererCalls.size() > 1)
	{
		rendererCalls.erase(rendererCalls.begin());
		needToRender = true;
		currentRenderer = rendererDB[rendererCalls[0]];
	}
	else if (rendererCalls.size() == 1)
	{
		rendererCalls.erase(rendererCalls.begin());
		currentRenderer = nullptr;
	}
}

void TextRenderer::render(sf::RenderWindow* surf)
{
	if (needToRender && currentRenderer != nullptr)
	{
		currentRenderer->render();
		currentRenderer->draw(surf);
		needToRender = false;
	}	
	else if (currentRenderer != nullptr)
	{
		currentRenderer->draw(surf);
	}
	else
	{
		std::cout << "<Error:Dialog:TextRenderer>[render] : No RenderClass has been specified for this TextRenderer" << std::endl;
	}
}

void Renderers::VisualNovel::load()
{
	name = "VisualNovel";

	sf::Texture* dialogLine = new sf::Texture;
	sf::Font* dialogFont = new sf::Font;
	anim::Animation* circleAnim = new anim::Animation;
	sf::Sprite* dialogLineSpr = new sf::Sprite;
	sf::Text* dialogText = new sf::Text;
	sf::Text* speakerText = new sf::Text;

	circleAnim->loadAnimation("Sprites/Dialog/Loader/", "Loader.ani.msd");
	circleAnim->playAnimation();
	circleAnim->getSprite()->setPosition(fn::Coord::width - 64 - 16, fn::Coord::height - 64 - 16);
	
	dialogLine->loadFromFile("Sprites/Dialog/textbox.png");
	dialogLineSpr->setTexture(*dialogLine);
	dialogLineSpr->setPosition(0, 760 * fn::Coord::height / fn::Coord::baseHeight);
	dialogFont->loadFromFile("Data/Fonts/TravelingTypewriter.ttf");
	
	dialogText->setFont(*dialogFont);
	dialogText->setCharacterSize(32);
	dialogText->setColor(sf::Color(255, 255, 255, 255));
	speakerText->setFont(*dialogFont);
	speakerText->setCharacterSize(48);
	speakerText->setColor(sf::Color(255, 255, 255, 255));

	this->locals["circleAnim"] = circleAnim;
	this->locals["dialogLineSpr"] = dialogLineSpr;
	this->locals["dialogText"] = dialogText;
	this->locals["speakerText"] = speakerText;
	this->locals["dialogLine"] = dialogLine;
	this->locals["dialogFont"] = dialogFont;
}

void Renderers::VisualNovel::unload()
{
	delete this->locals["circleAnim"].as<anim::Animation*>();
	delete this->locals["dialogLineSpr"].as<sf::Sprite*>();
	delete this->locals["dialogText"].as<sf::Text*>();
	delete this->locals["speakerText"].as<sf::Text*>();
	delete this->locals["dialogLine"].as<sf::Texture*>();
	delete this->locals["dialogFont"].as<sf::Font*>();
	if (this->locals.find("dispTex") != locals.end())
		delete this->locals["dispTex"].as<sf::Texture*>();
	if (this->locals.find("dispSpr") != locals.end())
		delete this->locals["dispSpr"].as<sf::Sprite*>();
}

void Renderers::VisualNovel::render()
{
	sf::Sprite* dialogLineSpr = this->locals["dialogLineSpr"].as<sf::Sprite*>();
	sf::Text* dialogText = this->locals["dialogText"].as<sf::Text*>();
	sf::Text* speakerText = this->locals["speakerText"].as<sf::Text*>();
	sf::Texture* dispTex = new sf::Texture;
	sf::Sprite* dispSpr = new sf::Sprite;

	renTex.clear(sf::Color(0, 0, 0, 200));

	renTex.draw(*dialogLineSpr);
	std::string speaker = vtdb[0]["speaker"];
	std::string textToSay = vtdb[0]["text"];
	speakerText->setString(sf::String(speaker));
	speakerText->setPosition((90 + (12 * (11 - speaker.size()))), 760 * fn::Coord::height / fn::Coord::baseHeight);
	renTex.draw(*speakerText);
	int indexCounter = 0;
	int lignAlign = 0;
	int borderSize = 30;
	std::string currentPhr;
	std::vector<std::string> currentTextList;
	if (fn::String::occurencesInString(textToSay, " ") >= 1)
		currentTextList = fn::String::split(textToSay, " ");
	else
		currentTextList = { textToSay };
	for (unsigned int i = 0; i < currentTextList.size(); i++)
	{
		std::string testPhr = currentPhr + currentTextList[i] + " ";
		fn::String::regenerateEncoding(testPhr);
		dialogText->setString(sf::String(testPhr));
		if (dialogText->getGlobalBounds().width > fn::Coord::width - (borderSize * 2))
			currentPhr += "\n";
		currentPhr += currentTextList[i] + " ";
		fn::String::regenerateEncoding(currentPhr);
		dialogText->setString(sf::String(currentPhr));
		indexCounter++;
	}
	fn::String::regenerateEncoding(currentPhr);
	dialogText->setString(sf::String(currentPhr));
	dialogText->setPosition(borderSize, 840 * fn::Coord::height / fn::Coord::baseHeight);
	renTex.draw(*dialogText);

	renTex.display();
	*dispTex = renTex.getTexture();
	dispSpr->setTexture(*dispTex);
	this->locals["dispTex"] = dispTex;
	this->locals["dispSpr"] = dispSpr;
	this->vtdb.erase(this->vtdb.begin());
}

void Renderers::VisualNovel::draw(sf::RenderWindow* surf)
{
	sf::Sprite* dispSpr = this->locals["dispSpr"].as<sf::Sprite*>();
	anim::Animation* circleAnim = this->locals["circleAnim"].as<anim::Animation*>();

	circleAnim->playAnimation();
	surf->draw(*dispSpr);
	surf->draw(*circleAnim->getSprite());
}

void Renderers::Shade::load()
{
	name = "Shade";

	sf::Font* dialogFont = new sf::Font;
	sf::Text* dialogText = new sf::Text;

	dialogFont->loadFromFile("Data/Fonts/TravelingTypewriter.ttf");

	dialogText->setFont(*dialogFont);
	dialogText->setCharacterSize(32);
	dialogText->setColor(sf::Color(255, 255, 255, 255));

	this->locals["dialogFont"] = dialogFont;
	this->locals["dialogText"] = dialogText;
}

void Renderers::Shade::unload()
{
	delete this->locals["dialogFont"].as<sf::Font*>();
	delete this->locals["dialogText"].as<sf::Text*>();
	if (this->locals.find("dispTex") != locals.end())
		delete this->locals["dispTex"].as<sf::Texture*>();
	if (this->locals.find("dispSpr") != locals.end())
		delete this->locals["dispSpr"].as<sf::Sprite*>();
}

void Renderers::Shade::render()
{
	sf::Text* dialogText = this->locals["dialogText"].as<sf::Text*>();
	sf::Texture* dispTex = new sf::Texture;
	sf::Sprite* dispSpr = new sf::Sprite;

	renTex.clear(sf::Color(0, 0, 0, 200));

	std::string textToSay = vtdb[0]["text"];
	int indexCounter = 0;
	int lignAlign = 0;
	int borderSize = 30;
	std::string currentPhr;
	std::vector<std::string> currentTextList;
	if (fn::String::occurencesInString(textToSay, " ") >= 1)
		currentTextList = fn::String::split(textToSay, " ");
	else
		currentTextList = { textToSay };
	for (unsigned int i = 0; i < currentTextList.size(); i++)
	{
		std::string testPhr = currentPhr + currentTextList[i] + " ";
		fn::String::regenerateEncoding(testPhr);
		dialogText->setString(sf::String(testPhr));
		if (dialogText->getGlobalBounds().width > fn::Coord::width - (borderSize * 2))
			currentPhr += "\n";
		currentPhr += currentTextList[i] + " ";
		fn::String::regenerateEncoding(currentPhr);
		dialogText->setString(sf::String(currentPhr));
		indexCounter++;
	}
	fn::String::regenerateEncoding(currentPhr);
	dialogText->setString(sf::String(currentPhr));
	dialogText->setPosition(borderSize, 540 * fn::Coord::height / fn::Coord::baseHeight);
	renTex.draw(*dialogText);

	renTex.display();
	*dispTex = renTex.getTexture();
	dispSpr->setTexture(*dispTex);
	this->locals["dispTex"] = dispTex;
	this->locals["dispSpr"] = dispSpr;
	this->vtdb.erase(this->vtdb.begin());
}

void Renderers::Shade::draw(sf::RenderWindow* surf)
{
	sf::Sprite* dispSpr = this->locals["dispSpr"].as<sf::Sprite*>();
	surf->draw(*dispSpr);
}

void Renderer::addTDB(std::map<std::string, std::string> tdb)
{
	vtdb.push_back(tdb);
}