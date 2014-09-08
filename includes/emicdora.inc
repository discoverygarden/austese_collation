<?php

/**
 * @file
 * Emicdora modifications.
 */
function collationtools_get_edited_collation($document_id) {
  module_load_include('inc', 'collation', 'includes/calliope');
  global $base_url;
  $file_query = array(
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
  $fixed_content = austesee_collation_fix_calliope($spans, $_GET['diff_kind']);
  $file_params = array(
    'collation' => $document_id,
    'version1' => $_GET['version1'],
    'version2' => $_GET['version2'],
    'diff_kind' => $_GET['diff_kind'],
    'emicdora_count' => 0,
  );
  emicdora_calliope_write_collation($file_params, $contents);
  echo $contents;
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
function austesee_collation_fix_calliope($spans, $type) {
  $repaired = array();
  foreach ($spans as $index => $span) {
    $span_pos = strpos(trim($span), '<span');
    if (strpos(trim($span), '<span') !== 0) {
      $repaired[$index] = "<span class='$type'>$span</span> ";
    }
    else {
      $repaired[$index] = "$span</span>";
    }
  }
  return implode('', $repaired);
}