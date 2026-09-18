#include "Viewport.h"
#include "FEditorViewportClient.h"

uint8 FViewport::getId() const
{
	return Id;
}
EViewportType FViewport::getType() const
{
	return Type;
}
FEditorViewportClient& FViewport::getClient()
{
	return *Client;
}
const FEditorViewportClient& FViewport::getClient() const
{
	return *Client;
}
FViewportWindowState& FViewport::getWindowState()
{
	return windowState;
}
const FViewportWindowState& FViewport::getWindowState() const
{
	return windowState;
}
void FViewport::setId(uint8 id)
{
	Id = id;
}
