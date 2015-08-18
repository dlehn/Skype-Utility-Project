$packageName = 'skype-utility-project'
$url = 'https://github.com/dlehn/Skype-Utility-Project/releases/download/1.4.0/skype_utility_project_binary_1.4.0.zip'
$skypeRegistry = "HKLM:\Software\Skype"
if (Test-Path HKLM:\Software\Wow6432Node\Skype)
{
    $skypeRegistry = "HKLM:\Software\Wow6432Node\Skype"
}

$skypeFolder = (Get-ItemProperty -Path ($skypeRegistry + "\Phone")).SkypeFolder

$process = Get-Process skype -ErrorAction SilentlyContinue
$wasOpen = $process -ne $null

if ($wasOpen)
{
    $success =  $process  | % { $_.CloseMainWindow() }
}
 
Install-ChocolateyZipPackage $packageName $url $skypeFolder
 
#Something like this would be cool, but you don't want to start it as a different user.
#if ($wasOpen)
#{
#    Start-Process (Get-ItemProperty -Path ($skypeRegistry + "\Phone")).SkypePath
#}