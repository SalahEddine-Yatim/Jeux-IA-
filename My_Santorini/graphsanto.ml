(* Compile:
ocamlopt graphics.cmxa unix.cmxa str.cmxa graphoth.ml -o graphoth
 *)

(*
The program returns as exit code (64+white-black)
If an error has occurred, 255 is returned.
It is thus possible to use it in scripts to set up tournaments by just
checking the exit code
 *)

(* 
In referee mode, no graphics are displayed and time controls
are strictly enforced: you are out of time => you lose 64-0 
*)

(* Options:
Usage: graph -fpn first_program -spn second_program -wait -time t
  -fpn First Program Name (default: no program)
  -spn Second Program Name (default: no program)
  -time Time for playing 20 moves in s (default: 60s)
  -wait Wait before closing graphic window (default: false)
  -pos the 2 white and 2 black workers positions as (5*x+y)
  -referee Sets referee mode (default: false)
  -help  Display this list of options
  --help  Display this list of options
 *)


let referee = ref false;;

let width=600 and height=600;;
let square=width/5;;
let full_height = height;;
let full_width= width;;

let curr_move = ref (-1,-1);;
let init_graph () =
  if (not !referee) then (
    let disp = try Sys.getenv("DISPLAY")
               with Not_found -> failwith "DISPLAY not set" in
    Graphics.open_graph (disp^" "^(string_of_int full_width)^"x"^
                           (string_of_int height));
    Graphics.auto_synchronize false);;

let tab = Array.make_matrix 5 5 0;;
let whites = [|(2,2);(3,3)|];;
let blacks = [|(3,2);(2,3)|];;

let affiche () =
  if (not !referee) then (
    let delta=9 and radius=25 in
    let grey = (Graphics.rgb 192 192 192) in
    let green = (Graphics.rgb 0 128 0) in
    let light = (Graphics.rgb 192 192 0) in
    let dark = (Graphics.rgb 0 0 96) in
    let light2 = (Graphics.rgb 255 255 0) in
    let dark2 = (Graphics.rgb 0 0 160) in
    Graphics.clear_graph ();
    Graphics.set_color grey;
    Graphics.fill_rect 0 0 full_width full_height;
    Graphics.set_color green;
    Graphics.fill_rect 0 0 width height;
    Graphics.set_color Graphics.black;
    for i=0 to 5 do
      Graphics.moveto 0 (i*square);
      Graphics.lineto width (i*square);
      Graphics.moveto (i*square) 0;
      Graphics.lineto (i*square) height;
    done;
    for i=0 to 4 do
      for j=0 to 4 do
        for k=1 to (min tab.(i).(j) 3) do
          let x = i*square+k*delta
          and y = j*square+k*delta
          and w = square-2*k*delta
          and h = square-2*k*delta in
          Graphics.set_color Graphics.white;
          Graphics.fill_rect x y w h;
          Graphics.set_color Graphics.black;
          Graphics.draw_rect x y w h
        done;
        if tab.(i).(j)=4 then (
          Graphics.set_color Graphics.red;
          Graphics.fill_circle (((2*i+1)*square)/2) (((2*j+1)*square)/2) radius
        );
      done;
    done;
    for ind=0 to 1 do
      Graphics.set_color light;
      let (i,j)=whites.(ind) in
      if (i,j)= !curr_move then Graphics.set_color light2; 
      Graphics.fill_circle (((2*i+1)*square)/2) (((2*j+1)*square)/2) radius;
      Graphics.set_color dark;
      let (i,j)=blacks.(ind) in
      if (i,j)= !curr_move then Graphics.set_color dark2; 
      Graphics.fill_circle (((2*i+1)*square)/2) (((2*j+1)*square)/2) radius;
    done;
    Graphics.synchronize()
  );;

exception Win;;
exception Ok;;
let check_win i j = if tab.(i).(j)=3 then raise Win;;
let has_moves i j =
  (*  Printf.printf "Has_moves:%d %d\n" i j;flush stdout;*)
  try
    for k= -1 to 1 do
      for l= -1 to 1 do
        if ((k<>0) || (l<>0)) && ((i+k)>=0)&&((i+k)<5)&&((j+l)>=0)&&((j+l)<5)&& 
             (tab.(i+k).(j+l)<=(tab.(i).(j)+1)) then
          try
            for d=0 to 1 do
              if (whites.(d)=(i+k,j+l))||
                   (blacks.(d)=(i+k,j+l)) then raise Exit;
            done;
            raise Ok;
          with Exit -> ()
      done;
    done;
    false
  with Ok -> true;;

let can_move x y x1 y1 =
  (*  Printf.printf "Can_move:%d %d %d %d\n" x y x1 y1;flush stdout;*)
  try
    if (x1<0)||(x1>4)||(y1<0)||(y1>4) then raise Exit;
    for d=0 to 1 do
      if ((x1,y1)=whites.(d))||((x1,y1)=blacks.(d)) then raise Exit;
    done;
    if ((abs (x-x1))>1)||((abs (y-y1))>1) then raise Exit;
    if tab.(x1).(y1)>(tab.(x).(y)+1) then raise Exit;
    true
  with Exit-> false;;

let can_build x y x1 y1 =
  (*  Printf.printf "Can_build:%d %d %d %d\n" x y x1 y1;flush stdout;*)
  try
    for d=0 to 1 do
      if ((x1,y1)=whites.(d))||((x1,y1)=blacks.(d)) then raise Exit;
    done;
    if ((abs (x-x1))>1)||((abs (y-y1))>1) then raise Exit;
    if tab.(x1).(y1)=4 then raise Exit;
    true
  with Exit-> false;;

exception Sortie of int;;
exception Sortie2 of int*int;;

let get_move turn =
  try
    while true do
      try
        affiche();
        let player = if turn then whites else blacks in
        let ind1 =
          try
            while true do
              let ev  = Graphics.wait_next_event
                          [Graphics.Button_down] in
              let x = ev.Graphics.mouse_x and y = ev.Graphics.mouse_y in
              let i = x/square and j=(y/square) in
              (*              Printf.printf "%d %d\n" i j;flush  stdout;*)
              for ind=0 to 1 do
                if ((i,j)=player.(ind)) && (has_moves i j) then raise (Sortie ind);
              done;
            done;
            failwith "Never"
          with Sortie i-> i in
        let (x1,y1)=player.(ind1) in
        curr_move := (x1,y1);
        affiche();
        
        let (x2,y2) =
          let ev  = Graphics.wait_next_event
                      [Graphics.Button_down] in
          let x = ev.Graphics.mouse_x and y = ev.Graphics.mouse_y in
          let i = x/square and j=(y/square) in
          (*          Printf.printf "%d %d\n" i j;flush  stdout;*)
          if (can_move x1 y1 i j) then (i,j)
          else (curr_move:=(-1,-1);raise Exit) in
        check_win x2 y2;
        player.(ind1)<-(x2,y2);
        curr_move:=(x2,y2);
        affiche();
        
        let (x3,y3) =
          let ev  = Graphics.wait_next_event
                      [Graphics.Button_down] in
          let x = ev.Graphics.mouse_x and y = ev.Graphics.mouse_y in
          let i = x/square and j=(y/square) in
          (*          Printf.printf "%d %d\n" i j;flush  stdout;*)
          if (can_build x2 y2 i j) then (i,j)
          else (player.(ind1)<-(x1,y1);curr_move:=(-1,-1);raise Exit) in
        player.(ind1)<-(x1,y1);
        curr_move:=(-1,-1);
        raise (Sortie ((5*x1+y1)+25*(x2*5+y2)+25*25*(5*x3+y3)))
      with Exit -> ()
    done;
    failwith "Never"
  with Sortie x -> x;;

let no_avail_moves turn=
  let player = if turn then whites else blacks in
  try
    for ind1 = 0 to 1 do
      let (x1,y1)=player.(ind1) in
      for x2= (max 0 (x1-1)) to (min 4 (x1+1)) do
        for y2= (max 0 (y1-1)) to (min 4 (y1+1)) do
          if (can_move x1 y1 x2 y2) then (
            if tab.(x2).(y2)=3 then raise Exit;
            player.(ind1)<-(x2,y2);
            for x3= (max 0 (x2-1)) to (min 4 (x2+1)) do
              for y3=(max 0 (y2-1)) to (min 4 (y2+1)) do
                if (can_build x2 y2 x3 y3) then (
                  player.(ind1)<-(x1,y1);
                  raise Exit);
              done;
            done;
            player.(ind1)<-(x1,y1);
          );
        done;
      done;
    done;
    true
  with Exit -> false;;
  
let start_prog name args turn =
  if name="" then (None,None,None)
  else (
    let (fd1_in,fd1_out) = Unix.pipe () 
    and (fd2_in,fd2_out) = Unix.pipe () in
    if turn then
      Printf.printf "Starting first program:%s\n" name
    else
      Printf.printf "Starting second program:%s\n" name;
    flush stdout;
    let pid = Unix.create_process name args fd1_in fd2_out Unix.stderr in
    Unix.close fd1_in;Unix.close fd2_out;
    let (fp1,fp2)= (Unix.in_channel_of_descr fd2_in,
                    Unix.out_channel_of_descr fd1_out) in
    (Some fp1,Some fp2,Some pid));;

let kill first_in first_out fpid sec_in sec_out spid =
  Unix.sleep 1;
  (match fpid with
   | None -> ()
   | Some pid ->
      let (Some first_in) = first_in and (Some first_out) = first_out in
      close_in first_in;close_out first_out;
      Printf.fprintf stderr "Killing %d\n" pid;flush stderr;
      (try Unix.kill pid Sys.sigkill with _ -> ());
      Printf.fprintf stderr "Waiting for %d\n" pid;flush stderr;
      ignore (Unix.waitpid [] pid));
  (match spid with
   | None -> ()
   | Some pid ->
      let (Some sec_in) = sec_in and (Some sec_out) = sec_out in
      close_in sec_in;close_out sec_out;
      Printf.fprintf stderr "Killing %d\n" pid;flush stderr;
      (try Unix.kill pid Sys.sigkill  with _ -> ());
      Printf.fprintf stderr "Waiting for %d\n" pid;flush stderr;
      ignore(Unix.waitpid [] pid));;

exception Invalid_origin;;
exception Invalid_destination;;
exception Invalid_build;;
let play x1 y1 x2 y2 x3 y3 turn =
  Printf.printf "From (%d,%d) to (%d,%d), build at (%d,%d)\n"
    x1 y1 x2 y2 x3 y3;
  flush stdout;
  let player = if turn then whites else blacks in
  let ind =
    try
      for d=0 to 1 do
        if player.(d)=(x1,y1) then raise (Sortie d);
      done;
      raise Invalid_origin;
    with Sortie ind -> ind in
  if not (can_move x1 y1 x2 y2) then raise Invalid_destination;
  player.(ind)<-(x2,y2);
  if tab.(x2).(y2)=3 then raise Win;
  if not (can_build x2 y2 x3 y3) then raise Invalid_build;
  tab.(x3).(y3)<-tab.(x3).(y3)+1;;

    
let _ =
  let turn = ref true in
  let fpn = ref "" and spn = ref ""
  and time = ref 60. and wait_close = ref false in
  let nb_pos=4 in
  let all_pos = Array.init nb_pos (fun _ -> ref "") in
  let make_lt t =
    let set mys s = 
      if s.[0]='-' then failwith "Error in argument list"
      else mys :=s in
    List.map (fun x-> (Arg.String (set x))) (Array.to_list t) in

  Arg.parse
    [
      ("-fpn",Arg.Set_string fpn,"First Program Name (default: no program)");
      ("-spn",Arg.Set_string spn,"Second Program Name (default: no program)");
      ("-time",Arg.Set_float time,"Time for 20 moves in s (default: 60s)");
      ("-pos",Arg.Tuple (make_lt all_pos),"Positions of the 2 white and 2 black workers as (5*x+y) (default: 12 18 13 17)");
      ("-referee",Arg.Set referee,"Referee mode (default: false)");
      ("-wait",Arg.Set wait_close,"Wait before closing graphic window (default: false)")
    ]
    (fun _ -> ())
    "Usage: graphsanto --help";
  if !referee && ((!fpn="")||(!spn="")) then
    failwith "referee mode is only possible with two programs";
  if !time < 1.0 then
    failwith "Minimal time for 20 moves is one second";
  let args1=
    if !(all_pos.(0))= "" then [|!fpn;"1";(string_of_float !time)|]
    else Array.append [|!fpn;"1";(string_of_float !time)|]
            (Array.map (fun x -> !x) all_pos) in
  let args2=
    if !(all_pos.(0))= "" then [|!spn;"2";(string_of_float !time)|]
    else Array.append [|!spn;"2";(string_of_float !time)|]
            (Array.map (fun x -> !x) all_pos) in
  let (first_in,first_out,fpid) = start_prog !fpn args1 true
  and (sec_in,  sec_out  ,spid) = start_prog !spn args2 false
  and nb_moves = ref 0 in
  if !(all_pos.(0))<>"" then (
    let tmp =
      Array.map
        (fun x-> let p = int_of_string !x in
                 let x=p/5 and y=p mod 5 in (x,y))
        all_pos in
    whites.(0)<-tmp.(0);whites.(1)<-tmp.(1);
    blacks.(0)<-tmp.(2);blacks.(1)<-tmp.(3);
  );
  try 
    let res =
      try
        let time_black = ref !time and time_white = ref !time in
        init_graph();
        while true do
          (*Printf.printf "Searching for available moves\n";flush stdout;*)
          if (no_avail_moves !turn) then (turn := not !turn;raise Win);
          (*Printf.printf "End searching for available moves\n";flush stdout;*)
          affiche ();
          let (fp1,fp2) =
            if !turn then (first_in,sec_out) else (sec_in,first_out) in
          let t = Unix.gettimeofday() in
          let move = 
            match fp1 with 
            | None ->
               Printf.printf "Getting move in board\n";
               flush stdout;
               get_move !turn
            | Some fp1 ->
               Printf.printf "Waiting for program\n";
               flush stdout;
               let st = input_line fp1 in
               int_of_string st in
          let pos = move mod 25 in
          let pnext = (move/25) mod 25 in
          let pbuild = move/25/25 in
          let (x1,y1,x2,y2,x3,y3) =
            (pos/5,pos mod 5,pnext/5,pnext mod 5,pbuild/5,pbuild mod 5) in
          let t = Unix.gettimeofday() -. t in
          if !turn then time_white := !time_white-. t
          else time_black := !time_black -. t;
          play x1 y1 x2 y2 x3 y3 !turn;
          turn := not !turn;
          if !referee && ((!time_white<0.)||(!time_black<0.)) then raise Win;
          match fp2 with
          | None -> ()
          | Some fp2 ->
             Printf.fprintf fp2 "%d\n" move;
             flush fp2;
             Printf.printf "Move %d sent to program\n" move;
             flush stdout
        done;
        failwith "never"
      with
        Win->
        affiche();
        Printf.printf 
          (if !turn then "White wins\n" else "Black Wins\n");
        flush stdout;
        if !turn then 192+ !nb_moves else 64+ !nb_moves in
    if !referee then (
      kill first_in first_out fpid sec_in sec_out spid;
      exit(res));
    if (!wait_close) then
      ignore (Graphics.wait_next_event [Graphics.Button_down]);
    kill first_in first_out fpid sec_in sec_out spid;
    exit(res);
  with
    x ->
     Printf.fprintf stderr "Exception:%s\n" (Printexc.to_string x);
     flush stderr;
    kill first_in first_out fpid sec_in sec_out spid;
    if !turn then exit 64 else exit 192;;

         
  

