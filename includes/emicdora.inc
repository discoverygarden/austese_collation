<?php

/**
 * @file
 * Emicdora modifications.
 */
function collationtools_get_edited_collation($document_id) {
  module_load_include('inc', 'collation', 'includes/calliope');
  global $base_url;
  $file_query = array(
    'emic_data.collation' => $document_id,
    'emic_data.version1' => $_GET['version1'],
    'emic_data.version2' => $_GET['version2'],
    'emic_data.diff_kind' => $_GET['diff_kind'],
  );
  $file = emicdora_calliope_get_collation_file($file_query);
  if ($file) {
    echo $file['emic_data']['contents'];
    drupal_exit();
  }
  $calliope_url = 'html/comparison';
  $path = "$base_url/$calliope_url/$document_id";
  $url = url($path, array('query' => array(
      '_dc' => $_GET['_dc'],
      'version1' => $_GET['version1'],
      'version2' => $_GET['version2'],
      'diff_kind' => $_GET['diff_kind'],
  )));
  $contents = file_get_contents($url);
  $spans = explode('</span>', $contents);
  $fixed_content = collationtools_fix_calliope($spans, $_GET['diff_kind']);

  $file_params = array(
    'collation' => $document_id,
    'version1' => $_GET['version1'],
    'version2' => $_GET['version2'],
    'diff_kind' => $_GET['diff_kind'],
    'emicdora_count' => 0,
  );
  emicdora_calliope_write_collation($file_params, $fixed_content);
  echo $fixed_content;
}

/**
 * Repairs possibly damaged Calliope.
 *
 * @param array $spans
 *   array of spans from file
 * @param string $type
 *   'delete' or 'added'
 *
 * @return xml
 *   Repaired xml
 */
function collationtools_fix_calliope($spans, $type) {
  $repaired = array();
  $repaired[] = array_shift($spans) . "</span>";
  $incomplete = FALSE;
  foreach ($spans as $span) {
    // Unpleasant but effective cure for mangled <br> tags coming from Calliope.
    if (substr_count($span, '<b')!= substr_count($span, '<br />')) {
      $span = str_replace('<b', '', $span);
      $incomplete = TRUE;
    }
    if ($incomplete && substr_count($span, ">r ") == 1) {
       $span = str_replace('>r', '>', $span);
       $incomplete = FALSE;
    }
    if (substr_count($span, ">/>") > 0) {
       $span = str_replace('>/>', '>', $span);
    }
    if (strpos(trim($span), '<span') !== 0) {
      $repaired[] = "<span class='$type'>$span</span> ";
    }
    else {
      $repaired[] = "$span</span>";
    }
  }
  // Add missing id's.
  $counter = 1;
  $fixed_xml = implode('', $repaired);
  $working_xml = "<div id = 'emicdora_wrapped'>$fixed_xml</div>";
  $dom = new DOMDocument();
  $dom->loadHTML($fixed_xml);
  $spans = $dom->getElementsByTagName('span');
  foreach ($spans as $span) {
    $id = $span->getAttribute('id');
    if ($id == '') {
      $id = $type . $counter;
      $span->setAttribute('id', $id);
      $counter++;
    }
  }
  return $dom->saveXML();
}
