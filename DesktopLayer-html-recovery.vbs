'Usage: me <possible-infected-file> <content-to-be-deleteed-that-stored-in-file>
'Also unicode encoded.

Option Explicit
Dim fso, oArgs, IsCLI

Set oArgs = WScript.Arguments
if oArgs.Count<>2 then WScript.Quit

Set fso = CreateObject("Scripting.FileSystemObject")
If LCase(Left(Right(WScript.FullName,11),7)) = "cscript" then IsCLI = True
call Main

Function msg(s)
    If IsCLI Then WScript.Echo s
End Function

Function ReadFileAll(F)
    Dim oStream
    Set oStream = CreateObject("ADODB.Stream")
    oStream.Open
    oStream.Type = 1 'binary
    oStream.LoadFromFile(F)
    ReadFileAll = oStream.Read(-1) 'all
    oStream.Close
End Function

Function WriteFileAll(F, S)
    On error resume next
    Dim oStream
    Set oStream = CreateObject("ADODB.Stream")
    oStream.Open()
    oStream.WriteText(S)
    oStream.SaveToFile F, 2 'Overwrite
    oStream.Close
    If Err.Number <> 0 Then
        msg "file	" & F & "	fail-write"
        Err.Clear
    Else
        msg "file	" & F & "	cleaned"
    End If
    On error Goto 0
End Function

Function AccessFile(F_Task, F_Del)
    Dim oF, sClean, sTask, sDel
    Dim len_t, len_d
    Dim infected
    '
    Set oF = fso.GetFile(F_Task)
    len_t = oF.Size
    len_d = fso.GetFile(F_Del).Size
    '
    If len_t > len_d then
        sTask = ReadFileAll(F_Task)
        sDel = ReadFileAll(F_Del)
        sClean = Replace(sTask, sDel, "")
        infected = Len(sTask) <> Len(sClean)
        if infected then
            WriteFileAll F_Task, sClean
        end if
    end If
    '
    if Not infected then
        msg "file	" & F_Task & "	not-infected"
    end if
End Function

Function AccessFolder(D_Task, F_Del)
    Dim oD
    Set oD = fso.GetFolder(D_Task)
    For each F in oD.Files
        AccessFile F.Path, F_Del
    Next
End Function

Function Main()
    Dim TASK, Vir

    TASK = oArgs(0)
    Vir = oArgs(1)

    If Not fso.FileExists(Vir) then
        msg "NotExist	" & Vir
        WScript.Quit
    end if

    If fso.FileExists(TASK) then
        AccessFile TASK, Vir
    end If

    If fso.FolderExists(TASK) then
        msg "folder	" & TASK
        AccessFolder TASK, Vir
    end If
End Function
