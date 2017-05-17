/*
    This file is part of Helio Workstation.

    Helio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Helio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Helio. If not, see <http://www.gnu.org/licenses/>.
*/

//[Headers]
#include "Common.h"
//[/Headers]

#include "AnnotationDialog.h"

//[MiscUserDefs]
#include "AnnotationsLayer.h"
#include "CommandIDs.h"

static StringPairArray getDynamics()
{
    StringPairArray c;
    c.set("Pianissimo",         Colours::greenyellow.toString());
    c.set("Piano",              Colours::gold.toString());
    c.set("Mezzo-piano",        Colours::tomato.toString());
    c.set("Mezzo-forte",        Colours::orangered.toString());
    c.set("Forte",              Colours::red.toString());
    c.set("Fortissimo",         Colours::fuchsia.toString());
    c.set("Al niente",          Colours::white.toString());
    c.set("Calmando",           Colours::royalblue.toString());
    c.set("Crescendo",          Colours::red.toString());
    c.set("Dal niente",         Colours::aqua.toString());
    c.set("Diminuendo",         Colours::blue.toString());
    c.set("Marcato",            Colours::lime.toString());
    c.set("Smorzando",          Colours::greenyellow.toString());
    return c;
}
//[/MiscUserDefs]

AnnotationDialog::AnnotationDialog(Component &owner, AnnotationsLayer *annotationsLayer, const AnnotationEvent &editedEvent, bool shouldAddNewEvent, float targetBeat)
    : targetEvent(editedEvent),
      targetLayer(annotationsLayer),
      ownerComponent(owner),
      addsNewEvent(shouldAddNewEvent),
      hasMadeChanges(false)
{
    addAndMakeVisible (background = new PanelC());
    addAndMakeVisible (panel = new PanelA());
    addAndMakeVisible (messageLabel = new Label (String(),
                                                 TRANS("...")));
    messageLabel->setFont (Font (Font::getDefaultSerifFontName(), 21.00f, Font::plain).withTypefaceStyle ("Regular"));
    messageLabel->setJustificationType (Justification::centred);
    messageLabel->setEditable (false, false, false);
    messageLabel->setColour (Label::textColourId, Colours::white);
    messageLabel->setColour (TextEditor::textColourId, Colours::black);
    messageLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (removeEventButton = new TextButton (String()));
    removeEventButton->setButtonText (TRANS("..."));
    removeEventButton->setConnectedEdges (Button::ConnectedOnTop);
    removeEventButton->addListener (this);

    addAndMakeVisible (shadow = new ShadowDownwards());
    addAndMakeVisible (okButton = new TextButton (String()));
    okButton->setButtonText (TRANS("..."));
    okButton->setConnectedEdges (Button::ConnectedOnTop);
    okButton->addListener (this);

    addAndMakeVisible (textEditor = new ComboBox (String()));
    textEditor->setEditableText (true);
    textEditor->setJustificationType (Justification::centredLeft);
    textEditor->setTextWhenNothingSelected (String());
    textEditor->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    textEditor->addListener (this);

    addAndMakeVisible (colourSwatches = new ColourSwatches());

    //[UserPreSize]
	jassert(this->addsNewEvent || this->targetEvent.getLayer() != nullptr);

	if (this->addsNewEvent)
	{
		Random r;
		const auto keys(getDynamics().getAllKeys());
		const String key(keys[r.nextInt(keys.size())]);
		const Colour colour(Colour::fromString(getDynamics()[key]));
		this->targetEvent = AnnotationEvent(annotationsLayer, targetBeat, key, colour);
		annotationsLayer->insert(this->targetEvent, true);

		this->messageLabel->setText(TRANS("dialog::annotation::add::caption"), dontSendNotification);
		this->okButton->setButtonText(TRANS("dialog::annotation::add::proceed"));
		this->removeEventButton->setButtonText(TRANS("dialog::annotation::add::cancel"));
	}
	else
	{
		this->messageLabel->setText(TRANS("dialog::annotation::edit::caption"), dontSendNotification);
		this->okButton->setButtonText(TRANS("dialog::annotation::edit::apply"));
		this->removeEventButton->setButtonText(TRANS("dialog::annotation::edit::delete"));
	}

	this->colourSwatches->setSelectedColour(this->targetEvent.getColour());

    this->textEditor->setText(this->targetEvent.getDescription(), dontSendNotification);
	this->textEditor->addItemList(getDynamics().getAllKeys(), 1);
	this->textEditor->addListener(this);
    //[/UserPreSize]

    setSize (450, 225);

    //[Constructor]
    this->rebound();
    this->setWantsKeyboardFocus(true);
    this->setInterceptsMouseClicks(true, true);
    this->toFront(true);
    this->setAlwaysOnTop(true);
    this->textEditor->grabKeyboardFocus();
    this->updateOkButtonState();

    this->startTimer(100);
    //[/Constructor]
}

AnnotationDialog::~AnnotationDialog()
{
    //[Destructor_pre]
    this->stopTimer();

    this->textEditor->removeListener(this);

    FadingDialog::fadeOut();
    //[/Destructor_pre]

    background = nullptr;
    panel = nullptr;
    messageLabel = nullptr;
    removeEventButton = nullptr;
    shadow = nullptr;
    okButton = nullptr;
    textEditor = nullptr;
    colourSwatches = nullptr;

    //[Destructor]
    //[/Destructor]
}

void AnnotationDialog::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.setColour (Colour (0x59000000));
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (getWidth() - 0), static_cast<float> (getHeight() - 0), 10.000f);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void AnnotationDialog::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    background->setBounds ((getWidth() / 2) - ((getWidth() - 10) / 2), 5, getWidth() - 10, getHeight() - 10);
    panel->setBounds ((getWidth() / 2) - ((getWidth() - 30) / 2), 15, getWidth() - 30, 153);
    messageLabel->setBounds ((getWidth() / 2) - ((getWidth() - 60) / 2), 5 + 16, getWidth() - 60, 36);
    removeEventButton->setBounds ((getWidth() / 2) + -5 - 150, 15 + 153, 150, 42);
    shadow->setBounds ((getWidth() / 2) - (310 / 2), 15 + 153 - 3, 310, 24);
    okButton->setBounds ((getWidth() / 2) + 5, 15 + 153, 150, 42);
    textEditor->setBounds ((getWidth() / 2) - ((getWidth() - 60) / 2), 62, getWidth() - 60, 36);
    colourSwatches->setBounds ((getWidth() / 2) - (384 / 2), 112, 384, 38);
    //[UserResized] Add your own custom resize handling here..
    this->textEditor->grabKeyboardFocus();
    //[/UserResized]
}

void AnnotationDialog::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == removeEventButton)
    {
        //[UserButtonCode_removeEventButton] -- add your button handler code here..
		if (this->addsNewEvent)
		{
			this->cancelAndDisappear();
		}
		else
		{
			this->removeEvent();
			this->disappear();
		}
        //[/UserButtonCode_removeEventButton]
    }
    else if (buttonThatWasClicked == okButton)
    {
        //[UserButtonCode_okButton] -- add your button handler code here..
		if (textEditor->getText().isNotEmpty())
		{
			this->disappear();
		}
        //[/UserButtonCode_okButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void AnnotationDialog::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == textEditor)
    {
        //[UserComboBoxCode_textEditor] -- add your combo box handling code here..
		this->updateOkButtonState();

		const String text(this->textEditor->getText());
        AnnotationEvent newEvent = this->targetEvent.withDescription(text);
        const String colourString(getDynamics()[text]);

        if (colourString.isNotEmpty())
        {
            const Colour c(Colour::fromString(colourString));
            this->colourSwatches->setSelectedColour(c);
            newEvent = newEvent.withColour(c);
        }

		if (text.isNotEmpty())
		{
			this->sendEventChange(newEvent);
		}
        //[/UserComboBoxCode_textEditor]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}

void AnnotationDialog::visibilityChanged()
{
    //[UserCode_visibilityChanged] -- Add your code here...
    this->textEditor->grabKeyboardFocus();
    //[/UserCode_visibilityChanged]
}

void AnnotationDialog::parentHierarchyChanged()
{
    //[UserCode_parentHierarchyChanged] -- Add your code here...
    this->rebound();
    //[/UserCode_parentHierarchyChanged]
}

void AnnotationDialog::parentSizeChanged()
{
    //[UserCode_parentSizeChanged] -- Add your code here...
    this->rebound();
    //[/UserCode_parentSizeChanged]
}

void AnnotationDialog::handleCommandMessage (int commandId)
{
    //[UserCode_handleCommandMessage] -- Add your code here...
    if (commandId == CommandIDs::DismissModalDialogAsync)
    {
		this->cancelAndDisappear();
	}
    //[/UserCode_handleCommandMessage]
}

bool AnnotationDialog::keyPressed (const KeyPress& key)
{
    //[UserCode_keyPressed] -- Add your code here...
    if (key.isKeyCode(KeyPress::escapeKey))
    {
		this->cancelAndDisappear();
		return true;
    }
    else if (key.isKeyCode(KeyPress::returnKey) ||
             key.isKeyCode(KeyPress::tabKey))
    {
		if (textEditor->getText().isNotEmpty())
		{
			this->disappear();
		}

        return true;
    }

    return false; // Return true if your handler uses this key event, or false to allow it to be passed-on.
    //[/UserCode_keyPressed]
}

void AnnotationDialog::inputAttemptWhenModal()
{
    //[UserCode_inputAttemptWhenModal] -- Add your code here...
    this->postCommandMessage(CommandIDs::DismissModalDialogAsync);
    //[/UserCode_inputAttemptWhenModal]
}


//[MiscUserCode]

AnnotationDialog *AnnotationDialog::createEditingDialog(Component &owner, const AnnotationEvent &event)
{
	return new AnnotationDialog(owner, static_cast<AnnotationsLayer *>(event.getLayer()), event, false, 0.f);
}

AnnotationDialog *AnnotationDialog::createAddingDialog(Component &owner, AnnotationsLayer *annotationsLayer, float targetBeat)
{
	return new AnnotationDialog(owner, annotationsLayer, AnnotationEvent(), true, targetBeat);
}

void AnnotationDialog::updateOkButtonState()
{
    const bool textIsEmpty = this->textEditor->getText().isEmpty();
    this->okButton->setAlpha(textIsEmpty ? 0.5f : 1.f);
    this->okButton->setEnabled(!textIsEmpty);
}

void AnnotationDialog::timerCallback()
{
    if (! this->textEditor->hasKeyboardFocus(true))
    {
        this->textEditor->grabKeyboardFocus();
        this->stopTimer();
    }
}

void AnnotationDialog::onColourButtonClicked(ColourButton *clickedButton)
{
	const Colour c(clickedButton->getColour());
    const AnnotationEvent newEvent =
		this->targetEvent.withDescription(this->textEditor->getText()).withColour(c);
	this->sendEventChange(newEvent);
}

void AnnotationDialog::sendEventChange(AnnotationEvent newEvent)
{
	if (this->targetLayer != nullptr)
	{
		this->cancelChangesIfAny();
		this->targetLayer->checkpoint();
		this->targetLayer->change(this->targetEvent, newEvent, true);
		this->hasMadeChanges = true;
	}
}

void AnnotationDialog::removeEvent()
{
	if (this->targetLayer != nullptr)
	{
		this->cancelChangesIfAny();
		this->targetLayer->checkpoint();
		this->targetLayer->remove(this->targetEvent, true);
		this->hasMadeChanges = true;
	}
}

void AnnotationDialog::cancelChangesIfAny()
{
	if (this->hasMadeChanges &&
		this->targetLayer != nullptr)
	{
		this->targetLayer->undo();
		this->hasMadeChanges = false;
	}
}

void AnnotationDialog::disappear()
{
	delete this;
}

void AnnotationDialog::cancelAndDisappear()
{
	this->cancelChangesIfAny(); // Discards possible changes

	if (this->addsNewEvent &&
		this->targetLayer != nullptr)
	{
		this->targetLayer->undo(); // Discards new event
	}

	this->disappear();
}
//[/MiscUserCode]

#if 0
/*
BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AnnotationDialog" template="../../Template"
                 componentName="" parentClasses="public FadingDialog, public TextEditorListener, public ColourButtonListener, private Timer"
                 constructorParams="Component &amp;owner, AnnotationsLayer *annotationsLayer, const AnnotationEvent &amp;editedEvent, bool shouldAddNewEvent, float targetBeat"
                 variableInitialisers="targetEvent(editedEvent),&#10;targetLayer(annotationsLayer),&#10;ownerComponent(owner),&#10;addsNewEvent(shouldAddNewEvent),&#10;hasMadeChanges(false)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="450" initialHeight="225">
  <METHODS>
    <METHOD name="parentSizeChanged()"/>
    <METHOD name="parentHierarchyChanged()"/>
    <METHOD name="keyPressed (const KeyPress&amp; key)"/>
    <METHOD name="visibilityChanged()"/>
    <METHOD name="inputAttemptWhenModal()"/>
    <METHOD name="handleCommandMessage (int commandId)"/>
  </METHODS>
  <BACKGROUND backgroundColour="0">
    <ROUNDRECT pos="0 0 0M 0M" cornerSize="10" fill="solid: 59000000" hasStroke="0"/>
  </BACKGROUND>
  <JUCERCOMP name="" id="e96b77baef792d3a" memberName="background" virtualName=""
             explicitFocusOrder="0" pos="0Cc 5 10M 10M" posRelativeH="ac3897c4f32c4354"
             sourceFile="../Themes/PanelC.cpp" constructorParams=""/>
  <JUCERCOMP name="" id="fee11f38ba63ec9" memberName="panel" virtualName=""
             explicitFocusOrder="0" pos="0Cc 15 30M 153" sourceFile="../Themes/PanelA.cpp"
             constructorParams=""/>
  <LABEL name="" id="cf32360d33639f7f" memberName="messageLabel" virtualName=""
         explicitFocusOrder="0" pos="0Cc 16 60M 36" posRelativeY="e96b77baef792d3a"
         textCol="ffffffff" edTextCol="ff000000" edBkgCol="0" labelText="..."
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default serif font" fontsize="21" kerning="0" bold="0"
         italic="0" justification="36"/>
  <TEXTBUTTON name="" id="ccad5f07d4986699" memberName="removeEventButton"
              virtualName="" explicitFocusOrder="0" pos="-5Cr 0R 150 42" posRelativeY="fee11f38ba63ec9"
              buttonText="..." connectedEdges="4" needsCallback="1" radioGroupId="0"/>
  <JUCERCOMP name="" id="ab3649d51aa02a67" memberName="shadow" virtualName=""
             explicitFocusOrder="0" pos="0Cc 3R 310 24" posRelativeY="fee11f38ba63ec9"
             sourceFile="../Themes/ShadowDownwards.cpp" constructorParams=""/>
  <TEXTBUTTON name="" id="7855caa7c65c5c11" memberName="okButton" virtualName=""
              explicitFocusOrder="0" pos="5C 0R 150 42" posRelativeY="fee11f38ba63ec9"
              buttonText="..." connectedEdges="4" needsCallback="1" radioGroupId="0"/>
  <COMBOBOX name="" id="1923d71c308d2169" memberName="textEditor" virtualName=""
            explicitFocusOrder="0" pos="0Cc 62 60M 36" editable="1" layout="33"
            items="" textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <JUCERCOMP name="" id="12e51c6931db61b4" memberName="colourSwatches" virtualName=""
             explicitFocusOrder="0" pos="0Cc 112 384 38" sourceFile="../Common/ColourSwatches.cpp"
             constructorParams=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif