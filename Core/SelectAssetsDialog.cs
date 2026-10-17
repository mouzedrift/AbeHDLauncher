using Godot;
using System.Linq;

namespace AbeHDLauncher.Core;

public partial class SelectAssetsDialog : Window
{
	[Export] private TextureRect _abePreview;
	[Export] private TextureRect _sligPreview;
	[Export] private Control _abeCheckBoxRoot;
	[Export] private Control _sligCheckBoxRoot;
	[Export] private TextureButton _exitButton;

	private ButtonGroup _abeButtonGroup = new();
	private ButtonGroup _sligButtonGroup = new();

	[Signal] public delegate void SelectionAcceptedEventHandler(Godot.Collections.Array<string> selectedAssetIds);


	// Called when the node enters the scene tree for the first time.
	public override void _Ready()
	{
		Hide();
		ForceNative = true;

		foreach (var checkBox in _abeCheckBoxRoot.GetChildren().OfType<CheckBox>())
		{
			checkBox.ButtonGroup = _abeButtonGroup;
		}

		foreach (var checkBox in _sligCheckBoxRoot.GetChildren().OfType<CheckBox>())
		{
			checkBox.ButtonGroup = _sligButtonGroup;
		}

		_abeButtonGroup.Pressed += (BaseButton button) =>
		{
			_abePreview.Texture = GD.Load<Texture2D>(button.GetMeta("image").AsString());
		};

		_sligButtonGroup.Pressed += (BaseButton button) =>
		{
			_sligPreview.Texture = GD.Load<Texture2D>(button.GetMeta("image").AsString());
		};

		CloseRequested += Hide;
		_exitButton.Pressed += () =>
		{
			Hide();
			EmitSignal(SignalName.SelectionAccepted, GetSelectedSkinFileIds());
		};
	}

	private Godot.Collections.Array<string> GetSelectedSkinFileIds()
	{
		return
		[
			_abeButtonGroup.GetPressedButton().GetMeta("file_id").AsString(),
		 	_sligButtonGroup.GetPressedButton().GetMeta("file_id").AsString()
		];
	}
}
