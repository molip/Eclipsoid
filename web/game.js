var output
var ws
var parser

var data = {}
data.teams = {} // team_id:data
data.current_team_id = null

data.current_action_elem = null
data.action = null
data.ping_timer_id = 0

var _capturer = new Capturer()

Colour = { 'Red': '#cd171a', 'Blue': '#283583', 'Green': '#3fa535', 'Yellow': '#ffd500', 'White': '#fff', 'Black': '#111' }

data.disc_imgs = {}
for (var col in Colour)
{
	data.disc_imgs[col] = new Image() 
	data.disc_imgs[col].src = "/images/discs/" + col + ".png"
}

data.ancient_img = new Image() 
data.ancient_img.src = "/images/ships/ancient ship.png"

data.gcds_img = new Image() 
data.gcds_img.src = "/images/ships/gcds_ship.png"

function GetCurrentTeam()
{
	return data.teams[data.current_team_id]
}

function GetFillColour(teamColour)
{
	return Colour[teamColour]
}

function GetStrokeColour(teamColour)
{
	return teamColour == 'Black' || teamColour == 'Blue' ? '#fff' : '#000'
}

function IsCurrentTeam(team_id)
{
	return team_id == data.current_team_id
}

function ExitAction(undo)
{
	if (data.action && data.action.CleanUp)
		data.action.CleanUp(undo)
	data.action = null
}

function LoadImage(path, onload)
{
	var img = new Image()
	img.src = path
	if (!img.complete)
		img.onload = onload
	return img
}

function Point(x, y)
{
	this.x = x
	this.y = y
}

Point.prototype.Clone = function()
{
	return new Point(this.x, this.y)
}

Point.prototype.equals = function(rhs)
{
	return rhs && this.x == rhs.x && this.y == rhs.y
}

function CompareObjects(lhs, rhs)
{
	return lhs == null ? rhs == null : lhs.equals(rhs)
}

function Assert(test, msg)
{
	if (!test)
		alert('Assert: ' + msg)
}

function IsTrue(str)
{
	return str == '1' || str == 'true';
}

function ShallowCopy(rhs) 
{
	var lhs = {}
	
	for (var v in rhs)
		lhs[v] = rhs[v]
	
	return lhs;
}

function GetChildElements(elem, name)
{
	var array = []
	for (var e = elem.firstChild; e; e = e.nextSibling)
		if (e.nodeName == name)
			array.push(e)
	return array
}

function GetFirstChildElement(elem, name)
{
	for (var e = elem.firstChild; e; e = e.nextSibling)
		if (e.nodeName == name)
			return e
	return null
}

function ClearSelect(select_elem)
{
	while (select_elem.length)
		select_elem.remove(0)
}

function OnMessage(msg)
{
	var obj = JSON.parse(msg.data);
	
	if (obj.command)
		OnCommand(obj.command)
	else if ('response' in obj)
		ShowBlanket(false)
}
  
function OnClose()
{
	ShowBlanket(false)
	document.getElementById('shroud').style.display = 'table' // For v centre

	if (data.ping_timer_id)
		window.clearInterval(data.ping_timer_id)
}

function load()
{
	document.getElementById('player_name_span').innerText = EscapeHtml(data.playerName)

	if ("WebSocket" in window)
	{
		var url = webSocketURL + '/' + data.playerID;
		ws = new WebSocket(url);
		ws.onopen = function() { SendRegister(); }
		ws.onmessage = OnMessage;
		ws.onclose = OnClose;

		data.ping_timer_id = window.setInterval(function() { ws.send('') }, 30000);
	}
	else
	{
		alert("WebSocket not supported by your Browser");
	}

	if (window.DOMParser)
		parser=new DOMParser();
	else
		alert("DOMParser not supported");
}

function ShowElementById(id, show)
{
	var e = document.getElementById(id)
	ShowElement(e, show)
	return e;
}

function ToggleElementById(id)
{
	var e = document.getElementById(id)
	if (e)
		e.style.display = e.style.display == 'none' ? '' : 'none' 
}

function ShowElement(e, show)
{
	if (e != null)
		e.style.display = show ? '' : 'none' // Can only override display set by inline style or JS - not CSS.
}

function IsElementVisible(id)
{
	var e = document.getElementById(id)
	return e.style.display != "none"
}

function ShowActionElement(id)
{
	ShowElement(data.current_action_elem, false)
	data.current_action_elem = id == null ? null : ShowElementById(id, true)
}

function ShowTeamPage(player_id, update)
{
	data.current_team_id = player_id

	if (update)
		Team.UpdateAll()

	ShowElementById('team_page', true)
	ShowElementById('supply_page', false)
}

function ShowTeamGeneral()
{
	ShowElementById('team_general', true)
	ShowElementById('team_blueprints', false)
}

function ShowTeamBlueprints()
{
	ShowElementById('team_general', false)
	ShowElementById('team_blueprints', true)
}

function ShowSupplyPage()
{
	ShowElementById('team_page', false)
	ShowElementById('supply_page', true)
}

function ShowBlanket(show)
{
	ShowElementById('blanket', show)
}

function ShowCombat(show)
{
	ShowElementById('combat', show)
	ShowElementById('map', !show)
	document.getElementById('map_ctrls').style.visibility = show ? 'hidden' : 'visible'
}

function ShowScore(show)
{
	ShowElementById('game_panel_score', show)
	ShowElementById('game_panel_play', !show)
}

function UpdateTeamTabs()
{
	for (var id in data.teams)
	{
		var button = document.getElementById('team_tab_{0}'.format(id)) 
		button.innerText = data.teams[id].name + (data.teams[id].is_waiting ? '*' : '')
	}
}


