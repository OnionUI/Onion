---
slug: /apps/search
---

# Search

*Search your game library*<sup><img align="left" src="https://user-images.githubusercontent.com/44569252/189498482-2590f31f-cca2-46e9-a316-3af98828446a.png" width="30" /></sup>


## Presentation

The Search application is the perfect companion for large game collections. It lets you find all the games containing the keyword you've entered.
Search also allows to filter a game list to include only games containing a specific keyword.

<table><thead>
<th colspan="2"><b>Search screenshots</b></th>
</thead><tr>
<td width="50%" align="center" valign="top"><img src="https://user-images.githubusercontent.com/44569252/189498639-8e2a43a6-4020-4492-b4b1-6e3f0c0d5fd6.png" width="320" /> </td>
<td width="50%" align="center" valign="top"><img src="https://user-images.githubusercontent.com/44569252/189498645-f615dd73-ed0c-4505-a439-5fb5b611237d.png" width="320" /> </td>
</tr><tr>
<td align="center" valign="top"><p><i>Pressing <kbd>X</kbd> will show Search app. Here you enter your keyword.</i></p></td>
<td align="center" valign="top"><p><i>A result example of search app.</i></p></td>
</tr></table>





## Usage

#### **Search**

- You'll find Search under `Apps` - and it's a popular choice for mapping to <kbd>X</kbd> (configured by default in Onion) or <kbd>Y</kbd> (via Tweaks)
- You can start search using the `X` button by default. You can change this keybinding in Tweaks
- When a search is active, the results will be shown under `Games` › `Search`, you can remove the search again by choosing "Clear search"
- "Missing caches" lists all systems which haven't been cached yet, and thereby haven't been included in the search
  - *Reason:* All game caches are cleared everytime you "Refresh all roms"
  - *Solution:* Go into each system you want included in the search


#### **Filter**

- *Filter* is accessible via the `GLO Menu` When in a game list, press <kbd>Y</kbd>
  - **Filter:** Will prompt you to enter a keyword for filtering the selected game list
  - In your filtered game list you will find 3 new options:
    - **~Clear filter:** Will remove the current filter to display your full game list again
    - **~Filter: "keyword":** Run filter app again to quickly modify your keywork
    - **~Refresh roms:** Will refresh roms only for the current filtered game list
- *Note: If you're using a `miyoogamelist.xml` you will need to add an entry for `~Filter.miyoocmd` and `~Refresh roms.miyoocmd`*


