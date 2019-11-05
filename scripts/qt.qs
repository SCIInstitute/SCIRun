function Controller() {
    installer.autoRejectMessageBoxes();
    installer.setMessageBoxAutomaticAnswer("installationError", QMessageBox.Retry);
    installer.setMessageBoxAutomaticAnswer("installationErrorWithRetry", QMessageBox.Retry);
    installer.setMessageBoxAutomaticAnswer("DownloadError", QMessageBox.Retry);
    installer.setMessageBoxAutomaticAnswer("archiveDownloadError", QMessageBox.Retry);
    installer.installationFinished.connect(function() {
        gui.clickButton(buttons.NextButton);
    })
}

Controller.prototype.WelcomePageCallback = function() {
    gui.clickButton(buttons.NextButton, 5000);
}

Controller.prototype.DynamicTelemetryPluginFormCallback = function() {
    gui.currentPageWidget().TelemetryPluginForm.statisticGroupBox.disableStatisticRadioButton.checked = true;
    gui.clickButton(buttons.NextButton, 3000);
}

Controller.prototype.CredentialsPageCallback = function() {
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.IntroductionPageCallback = function() {
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.TargetDirectoryPageCallback = function() {
    gui.currentPageWidget().TargetDirectoryLineEdit.setText(installer.value("InstallerDirPath") + "/Qt");
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.PerformInstallationPageCallback = function() {
    gui.clickButton(buttons.CommitButton);
}

Controller.prototype.ComponentSelectionPageCallback = function() {
    var selection = gui.pageWidgetByObjectName("ComponentSelectionPage");
    gui.findChild(selection, "Latest releases").checked = true;
    gui.findChild(selection, "LTS").checked = false;
    gui.findChild(selection, "FetchCategoryButton").click();

    var widget = gui.currentPageWidget();
    widget.deselectAll();

    if (installer.value("os") === "win") {
        widget.selectComponent("qt.qt5.5131.win64_msvc2017_64");
    } else if (installer.value("os") === "x11") {
        widget.selectComponent("qt.qt5.5131.gcc_64");
    } else if (installer.value("os") === "macos") {
        widget.selectComponent("qt.qt5.5131.clang_64");
    }

    widget.selectComponent("qt.tools.ifw.31");
    widget.selectComponent("qt.tools.openssl");
    widget.deselectComponent("qt.license");
    widget.deselectComponent("qt.installer");

    gui.clickButton(buttons.NextButton);
}

Controller.prototype.LicenseAgreementPageCallback = function() {
    gui.currentPageWidget().AcceptLicenseRadioButton.setChecked(true);
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.StartMenuDirectoryPageCallback = function() {
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.ReadyForInstallationPageCallback = function() {
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.FinishedPageCallback = function() {
    var checkBoxForm = gui.currentPageWidget().LaunchQtCreatorCheckBoxForm;
    if (checkBoxForm && checkBoxForm.launchQtCreatorCheckBox) {
        checkBoxForm.launchQtCreatorCheckBox.checked = false;
    }
    gui.clickButton(buttons.FinishButton);
}
